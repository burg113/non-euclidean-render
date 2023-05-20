#pragma once

#include <vector>

#include "../util/MathUtil.h"
#include "glm/matrix.hpp"

struct State{
    int tri;
    glm::vec2 pos, dir;
};

struct Triangle{
    glm::vec2 vert[3];

    // index i means side from i to i+1
    int nextTriangle[3];
    // add 3 if other direction (i+1 -> i)
    int nextSide[3];
    // when we leave the triangle at side i, apply the rotationMatrix to the direction
    // to obtain the corresponding direction in the new triangle
    glm::mat2 rotationMatrix[3];
    // when we leave the triangle at side i, newVert[i][0] corresponds to vert[i] and
    // newVert[i][1] corresponds to vert[(i+1)%3]
    glm::vec2 nextVert[3][2];

    // uv0 = uv at (0, 0); uvRight, uvUp = uv change when going right / up
    glm::vec2 uv0, uvRight, uvUp;

    // normal of the original
    glm::vec3 normal3d;

    // (distance, side)
    std::pair<float, short> rayIntersect(glm::vec2 pos, glm::vec2 dir);

    // a point somewhere in the middle (not on the border)
    glm::vec2 getMid();

    // flattens a 3d triangle a,b,c to 2d
    // maps a -> (0, 0), b -> (*, 0), c -> (*, *)
    void from3d(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec2 uvA, glm::vec2 uvB, glm::vec2 uvC);

    glm::vec2 getUV(glm::vec2 pos) const;
};

std::ostream& operator<<(std::ostream &stream, Triangle triangle);

struct GeoGraph{
    std::vector<Triangle> triangles;

    GeoGraph(std::vector<glm::vec3> vertices, std::vector<std::tuple<int, int, int>> triangleVerts,
             std::vector<glm::vec2> uv, std::vector<std::tuple<int, int, int>> triangleUV);
    std::pair<State, float> traverse(State state, float dist);
};

