#include <iostream>
#include <OpenImageIO/imageio.h>
#include "ImplicitVolumeShapes.h"
#include "BakedShadowMap.h"
#include "ImplicitColors.h"
#include "Camera.h"
#include "Raytracer.h"
#include "NoiseGenerator.h"
#include <omp.h>
#include <chrono>

using namespace std;
using namespace lux;
using namespace OIIO;

float ds = 0.025f;
float kappa = 1.0f;

float shm_ds = 0.015f;
float shm_kappa = 3.0f;

void Raytracer::write_exr(const vector<Color>& pixels, const Vector& screen_size, const int frame_number) const {
    const int width = static_cast<int>(screen_size[0]);
    const int height = static_cast<int>(screen_size[1]);
    const int channels = 4;

    char buf[64];
    std::snprintf(buf, sizeof(buf), "frame.%03d.exr", frame_number);
    std::string filename_str = buf;
    const char* filename = filename_str.c_str();
    vector<float> data(
        static_cast<size_t>(width) *
        static_cast<size_t>(height) *
        channels
    );

    for (size_t i = 0; i < pixels.size(); ++i) {
        data[i * 4 + 0] = pixels[i][0];
        data[i * 4 + 1] = pixels[i][1];
        data[i * 4 + 2] = pixels[i][2];
        data[i * 4 + 3] = pixels[i][3];
    }

    unique_ptr<ImageOutput> out = ImageOutput::create(filename);

    ImageSpec spec(width, height, channels, TypeDesc::FLOAT);
    spec.channelnames = {"R", "G", "B", "A"};

    if (!out->open(filename, spec)) {
        return;
    }

    if (!out->write_image(TypeDesc::FLOAT, data.data())) {
        cerr << "Error writing EXR: "
             << out->geterror() << endl;
        out->close();
        return;
    }

    out->close();
    cout << "Successfully wrote " << filename << " (frame " << frame_number << ")" << endl;
}

Color Raytracer::raymarch_one_pixel(const Vector& direction, const Camera& camera, const ScalarField& density, const ColorField& color, const vector<PointLight>& lights, const vector<ScalarField>& light_maps, const vector<float>& bounding_box) const {

    double T = 1;
    Color L(0,0,0,1);

    double start = camera.nearPlane();
    double end = camera.farPlane();
    
    // update based on bounding box
    if (bounding_box.size() == 6) {
        Vector eye = camera.eye();
        double tmin = (bounding_box[0] - eye[0]) / direction[0];
        double tmax = (bounding_box[1] - eye[0]) / direction[0];
        if (tmin > tmax) std::swap(tmin, tmax);

        double tymin = (bounding_box[2] - eye[1]) / direction[1];
        double tymax = (bounding_box[3] - eye[1]) / direction[1];
        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax)) {
            return Color(0, 0, 0, 1);
        }

        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        double tzmin = (bounding_box[4] - eye[2]) / direction[2];
        double tzmax = (bounding_box[5] - eye[2]) / direction[2];
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) {
            return Color(0, 0, 0, 1);
        }

        if (tzmin > tmin) tmin = tzmin;
        if (tzmax < tmax) tmax = tzmax;

        start = std::max(start, tmin);
        end = std::min(end, tmax);
    }

    double s = start + direction.magnitude() * ds * rand() / RAND_MAX;
    Vector X = camera.eye() + direction * s;
    while (s < end && T > 0.01) {
        float den = density->eval(X);
        if (den > 0.0) {
            Color Clights(0, 0, 0, 1);
            for (size_t i = 0; i < lights.size(); ++i) {
                Clights += lights[i].color * light_maps[i]->eval(X) * lights[i].intensity;
            }
            float dT = exp( -ds * kappa * den );
            L += color->eval(X) * Clights * (1 - dT) * T / kappa;
            T *= dT;
        }
        X += direction * ds;
        s += ds;
    }
    L[3] = 1.0;
    return L;
}

void Raytracer::raytrace(const Vector& screen_size, const Camera& camera, const ScalarField& density, const ColorField& color, int frame_number, const vector<PointLight>& lights, const vector<ScalarField>& shadow_maps, const vector<float>& bounding_box) const {

    const int width = static_cast<int>(screen_size[0]);
    const int height = static_cast<int>(screen_size[1]);

    vector<Color> pixels(width * height);
    
    auto before_time = std::chrono::steady_clock::now();
    #pragma omp parallel for collapse(2)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double u = static_cast<double>(x) / (width - 1);
            double v = 1.0 - static_cast<double>(y) / (height - 1);

            Vector ray_direction = camera.view(u, v);

            int index = y * width + x;
            pixels[index] = raymarch_one_pixel(ray_direction, camera, density, color, lights, shadow_maps, bounding_box);

        }
    }
    auto after_time = std::chrono::steady_clock::now();
    cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(after_time - before_time).count() << " seconds for raymarching" << endl;

    write_exr(pixels, screen_size, frame_number);
}

void Raytracer::circle_origin(const Vector& screen_size, const Camera& camera, const ScalarField& density, const ColorField& color, const int frames, const vector<PointLight>& lights, const vector<float>& bounding_box) const {
    
    Camera camera_copy = camera;

    vector<ScalarField> shadow_maps;

    auto before_time = std::chrono::steady_clock::now();
    for (const auto& light : lights) {
        BakedShadowMap* shadow_map = new BakedShadowMap(Vector(bounding_box[0], bounding_box[2], bounding_box[4]), Vector(bounding_box[1], bounding_box[3], bounding_box[5]), 0.05f, camera, shm_ds);
        shadow_map->bake(light, density);
        ScalarField ksm = new MultiplyVolume(shadow_map, -shm_kappa);
        ScalarField light_map = new ExpVolume(ksm);
        shadow_maps.push_back(light_map);
    }

    ScalarField density_field = new ClampVolume(density, 0.0f, 1.0f);

    auto after_time = std::chrono::steady_clock::now();
    cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(after_time - before_time).count() << " seconds for shadow maps" << endl;


    for (int i = 0; i < frames; ++i) {
        float angle = i * 3.0f * M_PI / 180.0f;
        camera_copy.setEyeViewUp(Vector(15.0f * sin(angle), -15.0f * cos(angle), 0.0f), Vector(-sin(angle), cos(angle), 0), Vector(0, 0, 1));
        raytrace(screen_size, camera_copy, density_field, color, i, lights, shadow_maps, bounding_box);
    }
}