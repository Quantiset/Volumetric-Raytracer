
#include <iostream>
#include <OpenImageIO/imageio.h>
#include "ImplicitVolumeShapes.h"
#include "ImplicitColors.h"
#include "Camera.h"
#include "Raytracer.h"
#include "LevelSet.h"
#include <omp.h>
#include <cmath>
#include <ctime>
#include <random>

using namespace std;
using namespace lux;
using namespace OIIO;

const int start_frame = 0;
const int num_frames = 120;

int main() {
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dis(0.0, 1.0);
    omp_set_num_threads(8); 

    Color brown(0.6f, 0.4f, 0.2f, 1.0f);
    Color red(1.0f, 0.0f, 0.0f, 1.0f);
    Color skin(0.8f, 0.6f, 0.4f, 1.0f);
    Color white(1.0f, 1.0f, 1.0f, 1.0f);
    Color yellow(1.0f, 1.0f, 0.0f, 1.0f);
    Color green(0.0f, 1.0f, 0.0f, 1.0f);

    Raytracer raytracer;

    Vector screen_size(960, 540, 0);

    const float Y_SIZE = 0.5;
    const int num_spheres_on_head = 20;
    vector<float> bounding_box = {-4.5, 4.5, -4.5, 4.5, -4.5, 4.5};

    Camera camera;
    camera.setEyeViewUp(Vector(0, -15, 0), Vector(0, 1, 0), Vector(0, 0, 1));
    camera.setFov(60.0);
    camera.setAspectRatio(static_cast<double>(screen_size[0]) / screen_size[1]);
    camera.setNearPlane(5.0);
    camera.setFarPlane(20.0);

    PointLight key_light(Vector(0, -2, 4), Color(1, 0.2, 0.2, 1), 2.0f);
    PointLight fill_light(Vector(-1.5, -0.5, -3), Color(0.2, 0.2, 1, 1), 0.4f);
    PointLight rim_light(Vector(0, -3, -0.5), Color(0.2, 1, 0.2, 1), 0.8f);
    
    vector<PointLight> lights = {key_light, fill_light, rim_light};

    vector<pair<ScalarField, Color>> volumes;


    for (int i = start_frame; i < start_frame + num_frames; ++i) {
        float j = (static_cast<float>(i) - start_frame) / num_frames;
        volumes.clear();

        ScalarField noise1 = new NoiseGenerator(0.5f, 4.2f - 3.0f * j, Vector(0.1f, 0.0f, 0.0f), 3.0f , 2.141f);
        ScalarField noise2 = new NoiseGenerator(0.5f, 3.0f - 2.0f * j, Vector(0.1f, 0.5f, 0.0f), 2.0f , 2.141f);

        time_t current_time = time(nullptr);
        ScalarField wisp = new Wisp(noise1, noise2, 45000000, 3.0f, 0.5f, 1.5f);
        time_t elapsed_time = time(nullptr) - current_time;
        cout << "Wisp grid generation took " << elapsed_time << " seconds." << endl;

        volumes.push_back(make_pair(wisp, white));

        auto [density_field, color_field] = raytracer.volumes_to_fields(volumes);
        raytracer.circle_origin(screen_size, camera, density_field, color_field, {i}, lights, bounding_box);
    }
}
