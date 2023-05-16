#include "MathUtil.h"
#include <cmath>

Vec3d::Vec3d(float x, float y, float z):x(x),y(y),z(z) {}
Vec3d Vec3d::Vec3d::Vec3d::cross(Vec3d other) const{
    return Vec3d(y*other.z - z*other.y,  z*other.x - x*other.z, x*other.y - y*other.x);
}
float Vec3d::dot(Vec3d other) const{
    return x*other.x + y*other.y + z*other.z;
}
float Vec3d::lenSq() const{
    return x*x + y*y + z*z;
}
float Vec3d::len() const{
    return sqrt(lenSq());
}
Vec3d Vec3d::normalized() const{
    return this->operator*(1.0f / len());
}
Vec3d Vec3d::operator* (float lambda) const{
    return Vec3d(lambda*x, lambda*y, lambda*z);
}
Vec3d Vec3d::operator+ (Vec3d other) const{
    return Vec3d(x+other.x, y+other.y, z+other.z);
}
Vec3d Vec3d::operator- (Vec3d other) const{
    return Vec3d(x-other.x, y-other.y, z-other.z);
}

Vec3d Vec3d::operator/(float lambda) const {
    return Vec3d(x/lambda, y/lambda, z/lambda);
}


Vec2d::Vec2d(float x, float y):x(x),y(y) {}
float Vec2d::cross(Vec2d other) const{
    return x*other.y - y*other.x;
}
float Vec2d::dot(Vec2d other) const{
    return x*other.x + y*other.y;
}
Vec2d Vec2d::perp() const{
    return Vec2d(-y, x);
}
float Vec2d::lenSq() const{
    return x*x + y*y;
}
float Vec2d::len() const{
    return sqrt(lenSq());
}
Vec2d Vec2d::normalized() const{
    return this->operator*(1.0f / len());
}
Vec2d Vec2d::operator+ (Vec2d other) const{
    return Vec2d(x+other.x, y+other.y);
}
Vec2d Vec2d::operator- (Vec2d other) const{
    return Vec2d(x-other.x, y-other.y);
}
Vec2d Vec2d::operator* (float lambda) const{
    return Vec2d(lambda*x, lambda*y);
}
Vec2d Vec2d::operator/(float lambda) const {
    return Vec2d(x/lambda, y/lambda);
}
Vec2d Vec2d::operator+=(Vec2d other) {
    x += other.x;
    y += other.y;
    return *this;
}

ostream& operator<<(ostream &stream, Vec3d v){
    return stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}
ostream& operator<<(ostream &stream, Vec2d v){
    return stream << "(" << v.x << ", " << v.y << ")";
}


Mat2d Mat2d::operator* (Mat2d other) const{
    return {this->operator*(other.v1), this->operator*(other.v2)};
}

Vec2d Mat2d::operator* (Vec2d other) const{
    return v1*other.x + v2*other.y;
}



pair<float, float> lineIntersect(Vec2d p, Vec2d v, Vec2d q, Vec2d w){
    Vec2d d = q-p;
    float div = w.x*v.y - w.y*v.x;
    float s = (w.x*d.y - w.y*d.x) / div;
    float t = (v.x*d.y - v.y*d.x) / div;
    return {s, t};
}