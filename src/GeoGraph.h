#pragma once

#include "MathUtil.h"

struct State{
    int tri;
    Vec2d pos, dir;
};

struct Triangle{
    Vec2d vert[3];
    // (triangle_id, orientation)
    // index i means side from i to i+1
    // add 3 if other direction (i+1 -> i)
    pair<int, short> shared[3];

    // (distance, side)
    pair<float, short> rayIntersect(Vec2d pos, Vec2d dir);

    Vec2d getMid();
};

struct GeoGraph{
    vector<Triangle> triangles;

    GeoGraph(vector<Vec3d> vertices, vector<tuple<int, int, int>> triangleVerts);
    State traverse(State state, float dist);
};

