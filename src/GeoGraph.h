#pragma once

#include "MathUtil.h"

struct State{
    int tri;
    Vec2d pos, dir;
};

struct Triangle{
    Vec2d vert[3];

    // index i means side from i to i+1
    int nextTriangle[3];
    // add 3 if other direction (i+1 -> i)
    int nextSide[3];
    Mat2d rotationMatrix[3];
    Vec2d nextVert[3][2];

    Vec3d normal3d;

    // (distance, side)
    pair<float, short> rayIntersect(Vec2d pos, Vec2d dir);

    Vec2d getMid();
};

ostream& operator<<(ostream &stream, Triangle triangle);

struct GeoGraph{
    vector<Triangle> triangles;

    GeoGraph(vector<Vec3d> vertices, vector<tuple<int, int, int>> triangleVerts);
    State traverse(State state, float dist);
};

