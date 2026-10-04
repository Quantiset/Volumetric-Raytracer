
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

    Vector screen_size(1920, 1080, 0);

    const float Y_SIZE = 0.5;
    const int num_spheres_on_head = 20;
    vector<float> bounding_box = {-4.5, 4.5, -4.5, 4.5, -4.5, 4.5};

    Camera camera;
    camera.setEyeViewUp(Vector(0, -15, 0), Vector(0, 1, 0), Vector(0, 0, 1));
    camera.setFov(60.0);
    camera.setAspectRatio(static_cast<double>(screen_size[0]) / screen_size[1]);
    camera.setNearPlane(5.0);
    camera.setFarPlane(20.0);

    vector<pair<ScalarField, Color>> volumes;

    // body
    ScalarField cornerl = new PlaneVolume(Vector(-1.3, 0, 0), Vector(-1, 0, -0.1));
    ScalarField cornerr = new PlaneVolume(Vector(1.3, 0, 0), Vector(1, 0, -0.1));
    ScalarField body1 = new BoxVolume(Vector(0, 0, 0), Vector(1.4, Y_SIZE, 1.4));
    ScalarField body2 = new BoxVolume(Vector(0, 0, 0), Vector(1.4, Y_SIZE, 1.4));
    body1 = new CutVolume(body1, cornerl);
    body1 = new CutVolume(body1, cornerr);
    body2 = new CutVolume(body2, cornerl);
    body2 = new CutVolume(body2, cornerr);
    ScalarField slice = new PlaneVolume(Vector(0, 0, 0), Vector(-0.5, 0, -1));
    ScalarField slice2 = new PlaneVolume(Vector(0, 0, 0), Vector(0.5, 0, 1));
    body1 = new CutVolume(body1, slice);
    body2 = new CutVolume(body2, slice2);

    // arms
    ScalarField arm1 = new BoxVolume(Vector(-1.65, 0, 0.6), Vector(0.4, Y_SIZE-0.1, 0.4));
    ScalarField arm2 = new BoxVolume(Vector(1.65, 0, 0.6), Vector(0.4, Y_SIZE-0.1, 0.4));
    
    volumes.push_back(make_pair(arm1, skin));
    volumes.push_back(make_pair(arm2, skin));
    
    ScalarField bat_cone = new ConeVolume(Vector(-1.65, 0, 0.75), Vector(0, 0, 1), 1.5, 0.4);
    ScalarField bat_end = new SphereVolume(Vector(-1.65, 0, 2.25), 0.375);
    ScalarField bat = new UnionVolume(bat_cone, bat_end);
    
    volumes.push_back(make_pair(bat, brown));

    ScalarField bangel = new TorusVolume(Vector(1.65, 0, 0.6), Vector(1, 0, 0), 0.65, 0.1);

    volumes.push_back(make_pair(bangel, yellow));

    // head
    ScalarField head = new BoxVolume(Vector(0, 0, 1.9), Vector(0.5, Y_SIZE - 0.1, 0.5));
    ScalarField blinn = new BlinnBlend(head, body1, 1.5, 1.5);
    
    for (int i = 0; i < num_spheres_on_head; ++i) {
        float angle = dis(gen) * 2.0f * M_PI;
        float x = 0.5f * cos(angle);
        float y = 0.4f * sin(angle);
        ScalarField sphere = new SphereVolume(Vector(x, y, 2.4), 0.15);
        volumes.push_back(make_pair(sphere, green));
    }
    ScalarField eye1 = new EllipseVolume(Vector(-0.2, -Y_SIZE, 2.1), Vector(0, 0, 1), 0.1, 0.05);
    ScalarField eye2 = new EllipseVolume(Vector(0.2, -Y_SIZE, 2.1), Vector(0, 0, 1), 0.1, 0.05);
    volumes.push_back(make_pair(eye1, white));
    volumes.push_back(make_pair(eye2, white));

    // feet
    ScalarField foot1 = new BoxVolume(Vector(-0.9, 0, -1.7), Vector(0.5, Y_SIZE, 0.5));
    ScalarField foot2 = new BoxVolume(Vector(0.9, 0, -1.7), Vector(0.5, Y_SIZE, 0.5));

    volumes.push_back(make_pair(foot1, skin));
    volumes.push_back(make_pair(foot2, skin));

    volumes.push_back(make_pair(head, skin));
    volumes.push_back(make_pair(blinn, skin));
    volumes.push_back(make_pair(body1, skin));
    volumes.push_back(make_pair(body2, white));


    const float statue_scale = 0.3f;
    const Vector statue_offset(1.65f, 0.0f, 1.30f + 3.61f * statue_scale);
    auto shrink_onto_hand = [&](const ScalarField& piece) {
        ScalarField scaled = new MultiplyVolume(ScalarField(new ScaleVolume(piece, statue_scale)), statue_scale);
        return ScalarField(new TranslateVolume(scaled, statue_offset));
    };

    LevelSet* ajax = new LevelSet(Vector(-2.6, -2.6, -3.8), Vector(2.6, 2.6, 1.0), 0.025f, 2);
    ajax->parse_obj("/home/ryon/Documents/Classes/cpsc8190/p2/models/ajax/smallajax.obj", 0.19f, YUp, Vector(-0.266f, 0.342f, -1.235f));
    ScalarField ajax_neck_plane = new PlaneVolume(Vector(-0.2, 0.8, -0.6), Vector(0, -0.567, 0.824));
    ScalarField headless_ajax = new CutVolume(ScalarField(ajax), ajax_neck_plane);
    volumes.push_back(make_pair(shrink_onto_hand(headless_ajax), white));

    LevelSet* bunny = new LevelSet(Vector(-4.5, -2.0, -2.5), Vector(3.0, 3.5, 4.2), 0.025f, 2);
    bunny->parse_obj("/home/ryon/Documents/Classes/cpsc8190/p2/models/bunny/bunny_fixed.obj", 55.0f, YUpFaceNegX, Vector(0.86f, -4.32f, 6.7325f));
    ScalarField bunny_neck_plane = new PlaneVolume(Vector(-0.2, 0.8, -0.6), Vector(0, 0.567, -0.824));
    ScalarField bunny_head = new CutVolume(ScalarField(bunny), bunny_neck_plane);
    volumes.push_back(make_pair(shrink_onto_hand(bunny_head), white));

    // render
    ScalarField density_field = new ConstantVolume(-99999.0f);
    ColorField color_field = new ConstantColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    for (int i = 0; i < volumes.size(); ++i) {
        ScalarField scaled_volume = new MultiplyVolume(volumes[i].first, 100.0f);
        ScalarField clamped_volume = new ClampVolume(scaled_volume, 0.0f, 1.0f);
        density_field = new UnionVolume(density_field, scaled_volume);
        ScalarField inv_mask = new AddVolume(
            ScalarField(new ConstantVolume(1.0f)),
            new MultiplyVolume(clamped_volume, -1.0f)
        );
        color_field = new AddColor(
            new MultiplyColor(color_field, inv_mask),
            new MultiplyColor(new ConstantColor(volumes[i].second), clamped_volume)
        );
    }

    PointLight key_light(Vector(0, -2, 4), Color(1, 0.2, 0.2, 1), 2.0f);
    PointLight fill_light(Vector(-1.5, -0.5, -3), Color(0.2, 0.2, 1, 1), 0.4f);
    PointLight rim_light(Vector(0, -3, -0.5), Color(0.2, 1, 0.2, 1), 0.8f);

    vector<PointLight> lights;
    lights.push_back(key_light);
    lights.push_back(fill_light);
    lights.push_back(rim_light);

    raytracer.circle_origin(screen_size, camera, density_field, color_field, num_frames, lights, bounding_box);
}
