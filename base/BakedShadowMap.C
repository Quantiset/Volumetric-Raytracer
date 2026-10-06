#include "BakedShadowMap.h"
#include "ImplicitColors.h"
#include "ImplicitVolumeShapes.h"
#include <omp.h>

using namespace std;
using namespace lux;

BakedShadowMap::BakedShadowMap( const Vector& min, const Vector& max, float res, Camera camera, float ds ) :
    Grid<float>(min, max, res), camera(camera), ds(ds)
{
}

BakedShadowMap::~BakedShadowMap() {
}

void BakedShadowMap::bake(const PointLight& light, const ScalarField& density_field) {

    ScalarField old_density_field = density_field;
    ScalarField new_density_field = new ClampVolume(density_field, 0.0f, 1.0f);

    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int grid_x = min_x; grid_x < min_x + nx; ++grid_x) {
        for (int grid_y = min_y; grid_y < min_y + ny; ++grid_y) {
            for (int grid_z = min_z; grid_z < min_z + nz; ++grid_z) {

                double arg = 0;
                Vector X = Vector(grid_x * res, grid_y * res, grid_z * res);
                float smax = (light.position - X).magnitude();
                Vector direction = (light.position - X).unitvector();
                double s = 0;
                if (old_density_field->eval(X) > -3.175f) {
                    while (s < smax) {
                        arg += ds * new_density_field->eval(X);
                        X += direction * ds;
                        s += ds;
                    }

                    set(grid_x, grid_y, grid_z, arg);
                }
            }
        }
    }
}