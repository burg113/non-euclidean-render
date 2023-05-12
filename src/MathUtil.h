#pragma once

#include <vector>
#include <tuple>
#include <cmath>
#include <map>
#include <iostream>

using namespace std;

constexpr float eps = 1e-5;

struct Vec3d{
    float x, y, z;

    Vec3d(float x=0, float y=0, float z=0);
    Vec3d cross(Vec3d other);
    float dot(Vec3d other);
    float lenSq();
    float len();
    Vec3d normalized();
    Vec3d operator* (float lambda);
    Vec3d operator+ (Vec3d other);
    Vec3d operator- (Vec3d other);
};

struct Vec2d{
    float x, y;

    Vec2d(float x=0, float y=0);
    float cross(Vec2d other);
    float dot(Vec2d other);
    Vec2d perp();
    float lenSq();
    float len();
    Vec2d normalized();
    Vec2d operator* (float lambda);
    Vec2d operator+ (Vec2d other);
    Vec2d operator- (Vec2d other);
};

ostream& operator<<(ostream &stream, Vec2d v);

struct Mat2d{
    Vec2d v1, v2;

    Mat2d operator* (Mat2d other);
    Vec2d operator* (Vec2d other);
};

// p, q positions, v, w directions
pair<float, float> lineIntersect(Vec2d p, Vec2d v, Vec2d q, Vec2d w);
