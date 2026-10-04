#pragma once

#include "Color.h"
#include "Vector.h"
#include "Camera.h"
#include "ImplicitVolumeShapes.h"
#include "ImplicitColors.h"
#include "PointLight.h"

using namespace lux;
using namespace std;

class Raytracer {

    public:
        Raytracer() = default;
        ~Raytracer() = default;

        void raytrace(const Vector& screen_size, const Camera& camera, const ScalarField& density, const ColorField& color, const int frame_index = 0, const vector<PointLight>& lights = vector<PointLight>(), const vector<ScalarField>& shadow_maps = vector<ScalarField>(), const vector<float>& bounding_box = vector<float>()) const;
        Color raymarch_one_pixel(const Vector& direction, const Camera& camera, const ScalarField& density, const ColorField& color, const vector<PointLight>& lights, const vector<ScalarField>& light_maps, const vector<float>& bounding_box) const;
        void write_exr(const std::vector<Color>& pixels, const Vector& screen_size, const int frame_number) const;

        void circle_origin(const Vector& screen_size, const Camera& camera, const ScalarField& density, const ColorField& color, const int frames, const vector<PointLight>& lights = vector<PointLight>(), const vector<float>& bounding_box = vector<float>()) const;
};