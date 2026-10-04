#include "Grid.h"
#include <cmath>

using namespace std;

namespace lux {

typedef shared_ptr<Grid<float>> GridVolume;

template<typename T>
Grid<T>::Grid( const Vector& _min, const Vector& _max, float _res, const T& _background ) : grid_data()
{
    res = _res;
    background = _background;
    allocate(_min, _max);
};

template<typename T>
void Grid<T>::allocate( const Vector& _min, const Vector& _max )
{
    min = _min;
    max = _max;

    min_x = static_cast<int>(floor(min.X() / res));
    min_y = static_cast<int>(floor(min.Y() / res));
    min_z = static_cast<int>(floor(min.Z() / res));

    nx = static_cast<int>(ceil(max.X() / res)) - min_x + 1;
    ny = static_cast<int>(ceil(max.Y() / res)) - min_y + 1;
    nz = static_cast<int>(ceil(max.Z() / res)) - min_z + 1;

    grid_data.assign(static_cast<size_t>(nx) * ny * nz, background);
}

template<typename T>
const T Grid<T>::eval( const Vector& P ) const {

    int grid_x = static_cast<int>(floor(P.X() / res));
    int grid_y = static_cast<int>(floor(P.Y() / res));
    int grid_z = static_cast<int>(floor(P.Z() / res));
    int grid_x_next = grid_x + 1;
    int grid_y_next = grid_y + 1;
    int grid_z_next = grid_z + 1;
    float frac_x = P.X() / res - grid_x;
    float frac_y = P.Y() / res - grid_y;
    float frac_z = P.Z() / res - grid_z;

    T val = get_grid_value(grid_x, grid_y, grid_z);
    T valx = get_grid_value(grid_x_next, grid_y, grid_z);
    T valy = get_grid_value(grid_x, grid_y_next, grid_z);
    T valz = get_grid_value(grid_x, grid_y, grid_z_next);
    T valxy = get_grid_value(grid_x_next, grid_y_next, grid_z);
    T valxz = get_grid_value(grid_x_next, grid_y, grid_z_next);
    T valyz = get_grid_value(grid_x, grid_y_next, grid_z_next);
    T valxyz = get_grid_value(grid_x_next, grid_y_next, grid_z_next);

    return val * (1 - frac_x) * (1 - frac_y) * (1 - frac_z) +
           valx * frac_x * (1 - frac_y) * (1 - frac_z) +
           valy * (1 - frac_x) * frac_y * (1 - frac_z) +
           valz * (1 - frac_x) * (1 - frac_y) * frac_z +
           valxy * frac_x * frac_y * (1 - frac_z) +
           valxz * frac_x * (1 - frac_y) * frac_z +
           valyz * (1 - frac_x) * frac_y * frac_z +
           valxyz * frac_x * frac_y * frac_z;


}

template<typename T>
void Grid<T>::stamp( const std::shared_ptr<Volume<T>>& volume ) { 
    stamp(*volume);
}

template<typename T>
void Grid<T>::stamp( const Volume<T>& volume ) {
    for (int x = min_x; x < min_x + nx; ++x) {
        for (int y = min_y; y < min_y + ny; ++y) {
            for (int z = min_z; z < min_z + nz; ++z) {
                set(x, y, z, volume.eval(Vector(x * res, y * res, z * res)));
            }
        }
    }
}

template class Grid<float>;

}