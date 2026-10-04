#include "LevelSet.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <limits>

using namespace std;
using namespace lux;

LevelSet::LevelSet( const Vector& min, const Vector& max, const float res, const int bandwidth ) :
    Grid<float>(min, max, res, -bandwidth * res), bandwidth(bandwidth)
{
}

LevelSet::~LevelSet() {
}

float LevelSet::point_to_triangle_distance(const Vector& P, const Vector& v0, const Vector& v1, const Vector& v2) {
    Vector edge0 = v1 - v0;
    Vector edge1 = v2 - v0;
    Vector v0_to_P = v0 - P;

    float a = edge0 * edge0;
    float b = edge0 * edge1;
    float c = edge1 * edge1;
    float d = edge0 * v0_to_P;
    float e = edge1 * v0_to_P;

    float det = a * c - b * b;
    float s = b * e - c * d;
    float t = b * d - a * e;

    if (s + t <= det) {
        if (s < 0) {
            if (t < 0) {
                if (d < 0) {
                    s = std::clamp(-d / a, 0.0f, 1.0f);
                    t = 0;
                } else {
                    s = 0;
                    t = std::clamp(-e / c, 0.0f, 1.0f);
                }
            } else {
                s = 0;
                t = std::clamp(-e / c, 0.0f, 1.0f);
            }
        } else if (t < 0) {
            s = std::clamp(-d / a, 0.0f, 1.0f);
            t = 0;
        } else {
            float invDet = 1.0f / det;
            s *= invDet;
            t *= invDet;
        }
    } else {
        if (s < 0) {
            float tmp0 = b + d;
            float tmp1 = c + e;
            if (tmp1 > tmp0) {
                float numer = tmp1 - tmp0;
                float denom = a - 2 * b + c;
                s = std::clamp(numer / denom, 0.0f, 1.0f);
                t = 1 - s;
            } else {
                t = std::clamp(-e / c, 0.0f, 1.0f);
                s = 0;
            }
        } else if (t < 0) {
            if (a + d > b + e) {
                float numer = c + e - b - d;
                float denom = a - 2 * b + c;
                s = std::clamp(numer / denom, 0.0f, 1.0f);
                t = 1 - s;
            } else {
                s = std::clamp(-e / c, 0.0f, 1.0f);
                t = 0;
            }
        } else {
            float numer = c + e - b - d;
            float denom = a - 2 * b + c;    
            s = std::clamp(numer / denom, 0.0f, 1.0f);
            t = 1 - s;
        }
    }
    return (v0 + edge0 * s + edge1 * t - P).magnitude();
}

void LevelSet::add_triangle(const Vector& v0, const Vector& v1, const Vector& v2) {
    
    float xmin = std::min({v0.X(), v1.X(), v2.X()});
    float xmax = std::max({v0.X(), v1.X(), v2.X()});
    float ymin = std::min({v0.Y(), v1.Y(), v2.Y()});
    float ymax = std::max({v0.Y(), v1.Y(), v2.Y()});
    float zmin = std::min({v0.Z(), v1.Z(), v2.Z()});
    float zmax = std::max({v0.Z(), v1.Z(), v2.Z()});

    int grid_xmin = static_cast<int>(floor(xmin / res));
    int grid_xmax = static_cast<int>(ceil(xmax / res)) ;
    int grid_ymin = static_cast<int>(floor(ymin / res)) ;
    int grid_ymax = static_cast<int>(ceil(ymax / res)) ;
    int grid_zmin = static_cast<int>(floor(zmin / res));
    int grid_zmax = static_cast<int>(ceil(zmax / res));

    for (int x = grid_xmin - bandwidth; x <= grid_xmax + bandwidth; ++x) {
        for (int y = grid_ymin - bandwidth; y <= grid_ymax + bandwidth; ++y) {
            for (int z = grid_zmin - bandwidth; z <= grid_zmax + bandwidth; ++z) {
                Vector P(x * res, y * res, z * res);
                float dist = LevelSet::point_to_triangle_distance(P, v0, v1, v2);
                float current_value = get_grid_value(x, y, z);
                if (current_value < 0 || dist < current_value) {
                    set(x, y, z, dist);
                }
            }

        }
    }

    for (int y = grid_ymin; y <= grid_ymax; ++y) {
        for (int z = grid_zmin; z <= grid_zmax; ++z) {
            Triangle t1;
            t1.v0 = v0;
            t1.v1 = v1;
            t1.v2 = v2;
            triangle_map[{y, z}].push_back(t1);
        }
    }


}

float LevelSet::ray_positive_x_to_triangle_distance(const Vector& P, const Vector& v0, const Vector& v1, const Vector& v2) {
    float ay = v1.Y() - v0.Y(), az = v1.Z() - v0.Z();
    float by = v2.Y() - v0.Y(), bz = v2.Z() - v0.Z();
    float ry = P.Y()  - v0.Y(), rz = P.Z()  - v0.Z();

    float det = ay * bz - by * az;
    if (fabs(det) < 1e-12f) return -1.0f;

    float u = (ry * bz - by * rz) / det;
    float v = (ay * rz - ry * az) / det;
    if (u < 0 || v < 0 || u + v > 1) return -1.0f;

    float hit_x = (1 - u - v) * v0.X() + u * v1.X() + v * v2.X();
    float d = hit_x - P.X();
    return (d >= 0) ? d : -1.0f;

}

void LevelSet::parse_obj(const string& filename, const float scale, const Orientation orientation, const Vector& translate) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    float min_x, max_x;
    float min_y, max_y;
    float min_z, max_z;
    min_x = 999999.0f;
    max_x = -999999.0f;
    min_y = 999999.0f;
    max_y = -999999.0f;
    min_z = 999999.0f;
    max_z = -999999.0f;

    vector<Vector> vertices;
    string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        string type;
        iss >> type;
        if (type == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            Vector v = Vector(x, y, z);
            if (orientation == YUp) {
                v = Vector(x, -z, y);
            } else if (orientation == YUpFaceNegX) {
                v = Vector(z, x, y);
            }
            v = v * scale - translate;

            x = v.X();
            y = v.Y();
            z = v.Z();
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
            min_z = std::min(min_z, z);
            max_z = std::max(max_z, z);
            vertices.push_back(v);
        } else if (type == "f") {
            vector<int> indices;
            string token;
            while (iss >> token) {
                int i = std::stoi(token.substr(0, token.find('/')));
                indices.push_back(i < 0 ? static_cast<int>(vertices.size()) + i : i - 1);
            }
            for (size_t k = 1; k + 1 < indices.size(); ++k) {
                add_triangle(vertices[indices[0]], vertices[indices[k]], vertices[indices[k + 1]]);
            }
        }
    }

    file.close();

    int grid_xmin = static_cast<int>(floor(min_x / res)) - bandwidth;
    int grid_xmax = static_cast<int>(ceil(max_x / res)) + bandwidth;
    int grid_ymin = static_cast<int>(floor(min_y / res)) - bandwidth;
    int grid_ymax = static_cast<int>(ceil(max_y / res)) + bandwidth;
    int grid_zmin = static_cast<int>(floor(min_z / res)) - bandwidth;
    int grid_zmax = static_cast<int>(ceil(max_z / res)) + bandwidth;

    for (int z = grid_zmin; z <= grid_zmax; ++z) {
        for (int y = grid_ymin; y <= grid_ymax; ++y) {

            Vector P(grid_xmin * res, y * res + 1.3e-4f * res, z * res + 0.7e-4f * res);
            vector<float> hits;
            for (const Triangle& t : triangle_map[{y, z}]) {
                float d = ray_positive_x_to_triangle_distance(P, t.v0, t.v1, t.v2);
                if (d >= 0) hits.push_back(P.X() + d);
            }
            std::sort(hits.begin(), hits.end());

            int index = 0;
            for (int x = grid_xmin; x <= grid_xmax; ++x) {
                float px = x * res;
                while (index < hits.size() && hits[index] < px) {
                    index++;
                }
                float current_value = get_grid_value(x, y, z);
                float mag = (current_value >= 0) ? current_value : bandwidth * res;
                set(x, y, z, (index % 2 == 1) ? mag : -mag); 
            }
            
        }
    }
}