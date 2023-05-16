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
    // when we leave the triangle at side i, apply the rotationMatrix to the direction
    // to obtain the corresponding direction in the new triangle
    Mat2d rotationMatrix[3];
    // when we leave the triangle at side i, newVert[i][0] corresponds to vert[i] and
    // newVert[i][1] corresponds to vert[(i+1)%3]
    Vec2d nextVert[3][2];

    // uv0 = uv at (0, 0); uvRight, uvUp = uv change when going right / up
    Vec2d uv0, uvRight, uvUp;

    // normal of the original
    Vec3d normal3d;

    // (distance, side)
    pair<float, short> rayIntersect(Vec2d pos, Vec2d dir);

    // a point somewhere in the middle (not on the border)
    Vec2d getMid();

    // flattens a 3d triangle a,b,c to 2d
    // maps a -> (0, 0), b -> (*, 0), c -> (*, *)
    void from3d(Vec3d a, Vec3d b, Vec3d c, Vec2d uvA, Vec2d uvB, Vec2d uvC);

    Vec2d getUV(Vec2d pos) const;
};

ostream& operator<<(ostream &stream, Triangle triangle);

struct GeoGraph{
    vector<Triangle> triangles;

    GeoGraph(vector<Vec3d> vertices, vector<tuple<int, int, int>> triangleVerts, vector<Vec2d> uv, vector<tuple<int, int, int>> triangleUV);
    pair<State, float> traverse(State state, float dist);
};

