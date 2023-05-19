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

    explicit Vec3d(float x=0, float y=0, float z=0);
    Vec3d cross(Vec3d other) const;
    float dot(Vec3d other) const;
    float lenSq() const;
    float len() const;
    Vec3d normalized() const;
    Vec3d operator+ (Vec3d other) const;
    Vec3d operator- (Vec3d other) const;
    Vec3d operator* (float lambda) const;
    Vec3d operator/ (float lambda) const;
};

struct Vec2d{
    float x, y;

    explicit Vec2d(float x=0, float y=0);
    float cross(Vec2d other) const;
    float dot(Vec2d other) const;
    Vec2d perp() const;
    float lenSq() const;
    float len() const;
    Vec2d normalized() const;
    Vec2d operator+ (Vec2d other) const;
    Vec2d operator- (Vec2d other) const;
    Vec2d operator* (float lambda) const;
    Vec2d operator/ (float lambda) const;
    Vec2d operator+=(Vec2d other);
};

ostream& operator<<(ostream &stream, Vec3d v);
ostream& operator<<(ostream &stream, Vec2d v);

struct Mat2d{
    Vec2d v1, v2;

    Mat2d operator* (Mat2d other) const;
    Vec2d operator* (Vec2d other) const;
};

// p, q positions, v, w directions
pair<float, float> lineIntersect(Vec2d p, Vec2d v, Vec2d q, Vec2d w);
