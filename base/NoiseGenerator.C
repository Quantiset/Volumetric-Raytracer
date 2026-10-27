#include "NoiseGenerator.h"
#include <cmath>

using namespace std;

float NoiseGenerator::perlin(Vector point) const {
    float x = point[0];
    float y = point[1];
    float z = point[2];
    int X = (int)std::floor(x) & 255,                  // FIND UNIT CUBE THAT
        Y = (int)std::floor(y) & 255,                  // CONTAINS POINT.
        Z = (int)std::floor(z) & 255;
    x -= std::floor(x);                                // FIND RELATIVE X,Y,Z
    y -= std::floor(y);                                // OF POINT IN CUBE.
    z -= std::floor(z);
    double u = fade(x),                                // COMPUTE FADE CURVES
            v = fade(y),                                // FOR EACH OF X,Y,Z.
            w = fade(z);
    int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
        B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,

    return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
                                    grad(p[BA  ], x-1, y  , z   )), // BLENDED
                            lerp(u, grad(p[AB  ], x  , y-1, z   ),  // RESULTS
                                    grad(p[BB  ], x-1, y-1, z   ))),// FROM  8
                    lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  // CORNERS
                                    grad(p[BA+1], x-1, y  , z-1 )), // OF CUBE
                            lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                                    grad(p[BB+1], x-1, y-1, z-1 ))));
}

float NoiseGenerator::noise(Vector point) const {
    Vector tpoint = point * scale_sample;
    int floor_octaves = static_cast<int>(n_octaves);
    float alpha = n_octaves - floor_octaves;
    float mult = 0.0f;
    for (int i = 0; i < floor_octaves; i++) {
        float r = 1.0f;
        float fj = 1.0f;
        for (int j = 0; j < i; j++) {
            r *= roughness;
            fj *= freq_jump;
        }
        mult += r * perlin((tpoint - translate) * freq * fj);
    }
    mult += alpha * pow(roughness, n_octaves - 1.0) * perlin((tpoint - translate) * freq * pow(freq_jump, n_octaves - 1.0));
    return (1.0f - roughness) * mult / (1.0f - pow(roughness, n_octaves));
}

const float NoiseGenerator::eval(const Vector& P) const {
    return noise(P);
}