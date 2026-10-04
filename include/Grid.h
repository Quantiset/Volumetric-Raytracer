#pragma once
#include "Volume.h"
#include "Vector.h"
#include <map>
#include <vector>

using namespace std;


namespace lux {

template<typename T>
class Grid : public Volume<T>
{
  public:

    Grid( const Vector& min, const Vector& max, float res, const T& background = T() );

    ~Grid(){}

    const T eval( const Vector& P ) const;

    void stamp( const std::shared_ptr<Volume<T>>& volume );
    void stamp( const Volume<T>& volume );
    void allocate( const Vector& min, const Vector& max );
    void set( const int x, const int y, const int z, const T& value ) {
        if (in_range(x, y, z)) {
            grid_data[index(x, y, z)] = value;
        }
    }

    protected:

    int min_x, min_y, min_z;
    int nx, ny, nz;
    Vector min;
    Vector max;
    float res;
    T background;

    std::vector<T> grid_data;

    bool in_range(int x, int y, int z) const
    {
        return x >= min_x && x < min_x + nx &&
               y >= min_y && y < min_y + ny &&
               z >= min_z && z < min_z + nz;
    }

    size_t index(int x, int y, int z) const
    {
        return (static_cast<size_t>(x - min_x) * ny + (y - min_y)) * nz + (z - min_z);
    }

    T get_grid_value(int x, int y, int z) const
    {
        return in_range(x, y, z) ? grid_data[index(x, y, z)] : background;
    }

};



}
