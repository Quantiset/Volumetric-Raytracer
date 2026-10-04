#pragma once
#include "Grid.h"
#include "Color.h"
#include "PointLight.h"
#include "Camera.h"

using namespace std;

namespace lux {

    struct Triangle {
        Vector v0, v1, v2;
    };

    enum Orientation {
        ZUp,
        YUp,
        YUpFaceNegX
    };

    class LevelSet: public Grid<float>
    {
      public:
        LevelSet( const Vector& min, const Vector& max, const float res, const int bandwidth = 2 );
        ~LevelSet();

        void add_triangle(const Vector& v0, const Vector& v1, const Vector& v2);
        void parse_obj(const std::string& filename, const float scale = 1.0f, const Orientation orientation = ZUp, const Vector& translate = Vector(0, 0, 0));
        float ray_positive_x_to_triangle_distance(const Vector& P, const Vector& v0, const Vector& v1, const Vector& v2);

        static float point_to_triangle_distance(const Vector& P, const Vector& v0, const Vector& v1, const Vector& v2);

      private:
        int bandwidth;
        map<pair<int, int>, vector<Triangle>> triangle_map;
    
    };
}