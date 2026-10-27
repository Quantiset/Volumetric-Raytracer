
#include "ImplicitVolumeShapes.h"
#include <iostream>
#include <cmath>
using namespace std;

using namespace lux;


ConstantVolume::ConstantVolume( const float v ) :
       value (v),
       gradvalue (Vector(0,0,0))
    {}


const float ConstantVolume::eval( const Vector& P ) const 
{
   return value; 
}

const Vector ConstantVolume::grad( const Vector& P ) const { return gradvalue; }


ExpVolume::ExpVolume( Volume<float>* v ) :
      elem (v)
    {}

ExpVolume::ExpVolume( const ScalarField& v ) :
      elem (v)
    {}

const float ExpVolume::eval( const Vector& P ) const 
{
   return std::exp( elem->eval(P) ); 
}

const Vector ExpVolume::grad( const Vector& P ) const { return eval(P) * elem->grad(P); }


const float IntersectVolume::eval( const Vector& P ) const {
   return min(elem1->eval(P), elem2->eval(P));
}

const Vector IntersectVolume::grad( const Vector& P ) const {
   if( elem1->eval(P) < elem2->eval(P) ){ return elem1->grad(P); }
   return elem2->grad(P);
}

const float CutVolume::eval( const Vector& P ) const {
   return min(elem1->eval(P), -elem2->eval(P));
}

const Vector CutVolume::grad( const Vector& P ) const {
   if( elem1->eval(P) < -elem2->eval(P) ){ return elem1->grad(P); }
   return -elem2->grad(P);
}

SphereVolume::SphereVolume( const Vector& cen, const float rad ) :
       center (cen),
       radius (rad)
    {}


const float SphereVolume::eval( const Vector& P ) const 
{
   Vector x = P-center;
   float rad = x.magnitude();
   return radius-rad;
}

const Vector SphereVolume::grad( const Vector& P ) const 
{
   Vector x = P-center;
   if( x.magnitude() != 0 ){ return -x.unitvector(); }
   return Vector(0,1,0);
}


AddVolume::AddVolume( const ScalarField&  v1, const ScalarField& v2 ) :
      e1 (v1),
      e2 (v2)
    {}

const float AddVolume::eval( const Vector& P ) const
{
   return  e1->eval(P) + e2->eval(P);
}

const Vector AddVolume::grad( const Vector& P ) const
{
   return  e1->grad(P) + e2->grad(P);
}


TranslateVolume::TranslateVolume( Volume<float>* v, const Vector& t ) : elem (v), trans (t) {

}

TranslateVolume::TranslateVolume( const ScalarField& v, const Vector& t ) : elem (v), trans (t) {

}

const float TranslateVolume::eval( const Vector& P ) const {
   return elem->eval(P-trans);
}
const Vector TranslateVolume::grad( const Vector& P ) const {
   return elem->grad(P-trans);
}

ScaleVolume::ScaleVolume( Volume<float>* v, const float s ) : elem (v), scale (s) {

}

ScaleVolume::ScaleVolume( const ScalarField& v, const float s ) : elem (v), scale (s) {

}

const float ScaleVolume::eval( const Vector& P ) const {
   return elem->eval(P/scale);
}
const Vector ScaleVolume::grad( const Vector& P ) const {
   return elem->grad(P/scale)/scale;
}


const float MaskVolume::eval( const Vector& P ) const {
   if( elem->eval(P) > 0.0f ){ return 1.0f; }
   return 0.0f;
}

const float ClampVolume::eval( const Vector& P ) const {
   float v = elem->eval(P);
   if( v < minValue ){ return minValue; }
   if( v > maxValue ){ return maxValue; }
   return v;
}

const float MultiplyVolume::eval( const Vector& P ) const {
   return elem1->eval(P) * elem2;
}

const Vector MultiplyVolume::grad( const Vector& P ) const {
   return elem1->grad(P) * elem2 + elem1->eval(P) * Vector(0, 0, 0);
}

const float BoxVolume::eval( const Vector& P ) const {
   Vector d = P - center;

   return 1.0f
      - std::pow(std::abs(d[0] / size[0]), exp)
      - std::pow(std::abs(d[1] / size[1]), exp)
      - std::pow(std::abs(d[2] / size[2]), exp);
}

const Vector BoxVolume::grad( const Vector& P ) const {
   Vector d = P - center;
   return d.unitvector();
}

const float EllipseVolume::eval( const Vector& P ) const {
   Vector x = P - center;
   float z = x * normal;
   Vector perp = x - z * normal;
   return 1-z*z / (major_axis*major_axis) - perp*perp / (minor_axis*minor_axis);
}

const Vector EllipseVolume::grad( const Vector& P ) const {
   Vector x = P - center;
   float z = x * normal;
   Vector perp = x - z * normal;
   return perp;
}

const float UnionVolume::eval( const Vector& P ) const {
   return max(elem1->eval(P), elem2->eval(P));
}

const Vector UnionVolume::grad( const Vector& P ) const {
   if( elem1->eval(P) > elem2->eval(P) ){ return elem1->grad(P); }
   return elem2->grad(P);
}

const float ConeVolume::eval( const Vector& P ) const {
   Vector x = P - center;
   if (x * normal > height) { 
      return height - x * normal; 
   } else if (x * normal < 0) { 
      return x * normal; 
   } else {
      float theta = atan(radius / height);
      return theta - acos((x * normal) / x.magnitude());
   }
}

const Vector ConeVolume::grad( const Vector& P ) const {
   return Vector(0, 0, 0);
}

const float PlaneVolume::eval( const Vector& P ) const {
   Vector x = P - center;
   return x * normal;
}

const Vector PlaneVolume::grad( const Vector& P ) const {
   return normal;
}

const float TorusVolume::eval( const Vector& P ) const {
   Vector x = P - center;
   float z = x * normal;
   Vector perp = x - z * normal;
   
   float left = 4 * major_axis * major_axis * (perp * perp);
   float right = (x * x) + (major_axis * major_axis) - (minor_axis * minor_axis);
   return left - right * right;
}

const Vector TorusVolume::grad( const Vector& P ) const {
   Vector x = P - center;
   float z = x * normal;
   Vector perp = x - z * normal;
   return perp;
}

const float BlinnBlend::eval( const Vector& P ) const {
   float d1 = elem1->eval(P);
   float d2 = elem2->eval(P);
   return std::exp(a1 * d1) + std::exp(a2 * d2) - 2;
}

const Vector BlinnBlend::grad( const Vector& P ) const {
   float d1 = elem1->eval(P);
   float d2 = elem2->eval(P);
   return a1 * std::exp(a1 * d1) * elem1->grad(P) + a2 * std::exp(a2 * d2) * elem2->grad(P);
} 

const float SteinerPatch::eval( const Vector& P ) const {
   Vector x = P - pos;
   float x2 = x[0] * x[0];
   float y2 = x[1] * x[1];
   float z2 = x[2] * x[2];
   return (x2 / (x2 + y2 + z2)) * (y2 / (x2 + y2 + z2)) * (z2 / (x2 + y2 + z2)) - (x[0] * x[1] * x[2]) / (x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
}

const Vector SteinerPatch::grad( const Vector& P ) const {
   Vector x = P - pos;
   return x;
}

const float PyroclasticDisplacement::eval( const Vector& P ) const {
   Vector on_surface = P - field->eval(P) * field->grad(P);
   return field->eval(P) + noise_generator->eval(on_surface) * amplitude;
}

const Vector PyroclasticDisplacement::grad( const Vector& P ) const {
   Vector on_surface = field->eval(P) * field->grad(P);
   return field->grad(P);
}

const float NoiseVolume::eval( const Vector& P ) const {
   return noise_generator->eval(P);
}

const Vector NoiseVolume::grad( const Vector& P ) const {
   return Vector(0, 0, 0);
}

const float Wisp::eval( const Vector& P ) const {
   return grid->eval(P);
}

void Wisp::initialize_grid() {
   random_device rd;
   mt19937 gen(rd());
   uniform_real_distribution<float> dis(-1.0, 1.0);

   for (int i = 0; i < num_children; ++i) {
      Vector square_point(dis(gen), dis(gen), dis(gen));
      Vector random_point = square_point / square_point.magnitude();
      float radius = std::pow(std::abs(noise_radius->eval(square_point)), clump);
      Vector scaled_pos = radius * random_point * pscale;
      Vector tmp(0.51, 0.21, 0.92);
      Vector displacement = Vector(noise_shift->eval(radius * random_point), noise_shift->eval(radius * random_point + tmp), noise_shift->eval(radius * random_point - tmp));
      Vector p5 = scaled_pos + displacement;
      grid->add(p5, density);
   }
}

const Vector Wisp::grad( const Vector& P ) const {
   return Vector(0, 0, 0);
}