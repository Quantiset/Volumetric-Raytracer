#pragma once
#include "Volume.h"
#include <unordered_map>
#include "Vector.h"
#include "Color.h"
#include <map>
#include <array>

using namespace std;


namespace lux {

class PointLight
{
  public:

    PointLight( const Vector& position, const Color& color, float intensity )
        : position(position), color(color), intensity(intensity) {

        }

    ~PointLight(){}

    Vector position;
    Color color;
    float intensity;


};

}