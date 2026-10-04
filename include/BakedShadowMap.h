#pragma once
#include "Grid.h"
#include "Color.h"
#include "PointLight.h"
#include "Camera.h"

using namespace std;

namespace lux {

  class BakedShadowMap: public Grid<float>
  {
    public:

      BakedShadowMap( const Vector& min, const Vector& max, float res, Camera camera, float ds );
      ~BakedShadowMap();

      void bake(const PointLight& light, const ScalarField& density_field);

    private:

      Camera camera;
      float ds;

  };

}