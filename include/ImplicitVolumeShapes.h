
#ifndef __IMPLICITVOLUMESHAPES_H__
#define __IMPLICITVOLUMESHAPES_H__

#include "Volume.h"
#include "LinearAlgebra.h"
#include <cmath>
#include <vector>
#include <iostream>
#include "Grid.h"


namespace lux
{


class ConstantVolume : public Volume<float> 
{
    public:

      ConstantVolume( const float v ) ;

    ~ConstantVolume(){}


      const float eval( const Vector& P ) const; 

      const Vector grad( const Vector& P ) const; 

    virtual std::string typelabel() { return "Constant"; }


    private:

      float value;
      Vector gradvalue;
  };


  class ExpVolume: public Volume<float> 
  {
    public:

      ExpVolume( Volume<float>* v );

      ExpVolume( const ScalarField& v );

    ~ExpVolume(){}


      const float eval( const Vector& P ) const; 

      const Vector grad(  const Vector& P ) const;

    virtual std::string typelabel() 
    { 
        std::string lbl = "Exp";
        lbl = lbl + "(";
        lbl = lbl + elem->typelabel();
        lbl = lbl + ")";
        return lbl;
    }

    private:

      const ScalarField elem;

  };




  class AddVolume : public Volume<float> 
  {
    public:

      AddVolume( Volume<float> * v1, Volume<float> * v2 );

      AddVolume( const ScalarField&  v1, const ScalarField& v2 );

      ~AddVolume(){}


      const float eval( const Vector& P ) const;

      const Vector grad(  const Vector& P ) const;

    virtual std::string typelabel() 
    { 
        std::string lbl = "Add";
        lbl = lbl + "(";
        lbl = lbl + e1->typelabel();
        lbl = lbl + ",";
        lbl = lbl + e2->typelabel();
        lbl = lbl + ")";
        return lbl;
    }

    private:

      const ScalarField e1, e2;
  };





  class SphereVolume : public Volume<float> 
  {
    public:

      SphereVolume( const Vector& cen, const float rad );

    ~SphereVolume(){}


      const float eval( const Vector& P ) const; 

      const Vector grad(  const Vector& P ) const; 

      virtual std::string typelabel() { return std::string("Sphere"); }


    private:

      Vector center;
      float radius;
  };

  class BoxVolume : public Volume<float> 
  {
    public:

      BoxVolume( const Vector& cen, const Vector& sz, const float e = 10.0f ) :
            center (cen),
            size (sz),
            exp (e)
          {}

      ~BoxVolume(){}

      const float eval( const Vector& P ) const;

      const Vector grad(  const Vector& P ) const;

      virtual std::string typelabel() { return std::string("Box"); }

    private:
    
      Vector center;
      Vector size;
      float exp;
  };

  class TranslateVolume : public Volume<float> 
  {
    public:

      TranslateVolume( Volume<float>* v, const Vector& t );

      TranslateVolume( const ScalarField& v, const Vector& t );

    ~TranslateVolume(){}
    
      const float eval( const Vector& P ) const; 

      const Vector grad(  const Vector& P ) const;

    private:
    
      const ScalarField elem;
      Vector trans;
  };

  class ScaleVolume : public Volume<float> 
  {
    public:

      ScaleVolume( Volume<float>* v, const float s );

      ScaleVolume( const ScalarField& v, const float s );

    ~ScaleVolume(){}
    
      const float eval( const Vector& P ) const; 

      const Vector grad(  const Vector& P ) const;

    private:
    
      const ScalarField elem;
      float scale;
  };

  class MaskVolume : public Volume<float> 
  {
    public:

      MaskVolume( Volume<float>* v ): 
        elem(v)
      {};

      MaskVolume( const ScalarField& v ): 
        elem(v)
      {};

    ~MaskVolume(){}
    
      const float eval( const Vector& P ) const; 


    private:
    
      const ScalarField elem;
  };

  class ClampVolume : public Volume<float> 
  {
    public:

      ClampVolume( Volume<float>* v, const float minVal, const float maxVal ): 
        elem(v), minValue(minVal), maxValue(maxVal)
      {};

      ClampVolume( const ScalarField& v, const float minVal, const float maxVal ): 
        elem(v), minValue(minVal), maxValue(maxVal)
      {};

    ~ClampVolume(){}
    
      const float eval( const Vector& P ) const; 

    private:
    
      const ScalarField elem;
      float minValue;
      float maxValue;

  };

  class MultiplyVolume : public Volume<float> 
  {
    public:

      MultiplyVolume( Volume<float>* v1, float v2 ): 
        elem1(v1), elem2(v2)
      {};

      MultiplyVolume( const ScalarField& v1, float v2 ): 
        elem1(v1), elem2(v2)
      {};

    ~MultiplyVolume(){}
    
      const float eval( const Vector& P ) const; 

      const Vector grad(  const Vector& P ) const;

    private:
    
      const ScalarField elem1;
      float elem2;
  };

  class EllipseVolume : public Volume<float> 
  {
    public:

      EllipseVolume( const Vector& cen, const Vector& norm, const float major, const float minor ) :
        center (cen),
        major_axis (major),
        minor_axis (minor),
        normal (norm.unitvector())
      {}

      ~EllipseVolume(){};

      const float eval( const Vector& P ) const;

      const Vector grad(  const Vector& P ) const;

      virtual std::string typelabel() { return std::string("Ellipse"); }

    private:
    
      Vector center;
      Vector normal;
      float major_axis;
      float minor_axis;
  };

  class UnionVolume : public Volume<float> 
  {
    public:

      UnionVolume( Volume<float>* v1, Volume<float>* v2 ): 
        elem1(v1), elem2(v2)
      {};

      UnionVolume( const ScalarField& v1, const ScalarField& v2 ): 
        elem1(v1), elem2(v2)
      {};

    ~UnionVolume(){}
    
      const float eval( const Vector& P ) const; 

      const Vector grad(  const Vector& P ) const;

    private:
    
      const ScalarField elem1;
      const ScalarField elem2;
  }; 

  class ConeVolume : public Volume<float> 
  {
    public:

      ConeVolume( const Vector& cen, const Vector& norm, const float height, const float radius ) :
        center (cen),
        height (height),
        radius (radius),
        normal (norm.unitvector())
      {}

      ~ConeVolume(){};

      const float eval( const Vector& P ) const;

      const Vector grad(  const Vector& P ) const;

      virtual std::string typelabel() { return std::string("Cone"); }

    private:
    
      Vector center;
      Vector normal;
      float height;
      float radius;

  };

  class PlaneVolume : public Volume<float> 
  {
    public:

      PlaneVolume( const Vector& cen, const Vector& norm ) :
        center (cen),
        normal (norm.unitvector())
      {}

      ~PlaneVolume(){};

      const float eval( const Vector& P ) const;

      const Vector grad(  const Vector& P ) const;

      virtual std::string typelabel() { return std::string("Plane"); }

    private:
    
      Vector center;
      Vector normal;

  };

  class IntersectVolume : public Volume<float> 
  {
    public:

      IntersectVolume( Volume<float>* v1, Volume<float>* v2 ): 
        elem1(v1), elem2(v2)
      {};

      IntersectVolume( const ScalarField& v1, const ScalarField& v2 ): 
        elem1(v1), elem2(v2)
      {};

    ~IntersectVolume(){}
    
      const float eval( const Vector& P ) const; 

      const Vector grad(  const Vector& P ) const;

    private:
    
      const ScalarField elem1;
      const ScalarField elem2;
  };

  class CutVolume : public Volume<float> 
  {
    public:

      CutVolume( Volume<float>* v1, Volume<float>* v2 ): 
        elem1(v1), elem2(v2)
      {};

      CutVolume( const ScalarField& v1, const ScalarField& v2 ): 
        elem1(v1), elem2(v2)
      {};

    ~CutVolume(){}
    
      const float eval( const Vector& P ) const; 

      const Vector grad(  const Vector& P ) const;

    private:
    
      const ScalarField elem1;
      const ScalarField elem2;
  };

  class TorusVolume : public Volume<float> 
  {
    public:

      TorusVolume( const Vector& cen, const Vector& norm, const float major, const float minor ) :
        center (cen),
        major_axis (major),
        minor_axis (minor),
        normal (norm.unitvector())
      {}

      ~TorusVolume(){};

      const float eval( const Vector& P ) const;

      const Vector grad(  const Vector& P ) const;

      virtual std::string typelabel() { return std::string("Torus"); }

    private:
    
      Vector center;
      Vector normal;
      float major_axis;
      float minor_axis;
  };

  class BlinnBlend : public Volume<float> 
  {
    public:

      BlinnBlend( Volume<float>* v1, Volume<float>* v2, const float _a1, const float _a2 ): 
        elem1(v1), elem2(v2), a1(_a1), a2(_a2)
      {};

      BlinnBlend( const ScalarField& v1, const ScalarField& v2, const float _a1, const float _a2 ): 
        elem1(v1), elem2(v2), a1(_a1), a2(_a2)
      {};

    ~BlinnBlend(){}
    
      const float eval( const Vector& P ) const; 

      const Vector grad(  const Vector& P ) const;

    private:
    
      const ScalarField elem1;
      const ScalarField elem2;
      float a1, a2;
  };

  class SteinerPatch : public Volume<float> 
  {
    public:

      SteinerPatch( const Vector position, const float x, const float y, const float z) :
        pos(position), a(x), b(y), c(z)
      {};

      ~SteinerPatch(){};

      const float eval( const Vector& P ) const;

      const Vector grad(  const Vector& P ) const;

    private:
    
      Vector pos;
      float a, b, c;

  };


};




#endif



