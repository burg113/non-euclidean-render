#include "MathUtil.h"
#include <cmath>

using namespace std;
using glm::vec2, glm::vec3;

ostream& operator<<(ostream &stream, vec3 v){
    return stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}
ostream& operator<<(ostream &stream, vec2 v){
    return stream << "(" << v.x << ", " << v.y << ")";
}

vec2 perp(const vec2& v) {
    return vec2(-v.y, v.x);
}

pair<float, float> lineIntersect(vec2 p, vec2 v, vec2 q, vec2 w){
    vec2 d = q-p;
    float div = w.x*v.y - w.y*v.x;
    float s = (w.x*d.y - w.y*d.x) / div;
    float t = (v.x*d.y - v.y*d.x) / div;
    return {s, t};
}