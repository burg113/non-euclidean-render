
#include <vector>
#include <tuple>
#include <map>

#include "MathUtil.h"
#include "GeoGraph.h"

using namespace std;

pair<float, short> Triangle::rayIntersect(Vec2d pos, Vec2d dir){
    int side = -1;
    float minDist = 1e8;
    for(int i = 0; i < 3; i++){
        float dist = lineIntersect(pos, dir, vert[i], vert[(i+1)%3]-vert[i]).first;
        if(dist > eps && dist < minDist){
            minDist = dist;
            side = i;
        }
    }
    return {minDist, side};
}


GeoGraph::GeoGraph(vector<Vec3d> vertices, vector<tuple<int, int, int>> triangleVerts) {
    int n = (int)triangleVerts.size();
    triangles.resize(n);

    map<pair<int, int>, pair<int, int>> byEdge;
    auto processEdge = [&](int i, int j, int tri, int side){
        auto p1 = make_pair(i, j);
        auto p2 = make_pair(j, i);
        if(byEdge.count(p1)){
            auto [tri2, side2] = byEdge[p1];
            triangles[tri].shared[side] = {tri2, side2};
            triangles[tri2].shared[side2] = {tri, side};
        }
        else if(byEdge.count((p2))){
            auto [tri2, side2] = byEdge[p2];
            triangles[tri].shared[side] = {tri2, side2+3};
            triangles[tri2].shared[side2] = {tri, side+3};
        }
        else{
            byEdge[p1] = {tri, side};
        }
    };

    for(int i = 0; i < n; i++){
        auto [ind0, ind1, ind2] = triangleVerts[i];
        // flatten triangle a,b,c to 2d
        // map a -> (0, 0), b -> (*, 0), c -> (*, *)
        Vec3d a3 = vertices[ind0];
        Vec3d b3 = vertices[ind1];
        Vec3d c3 = vertices[ind2];
        Vec3d aToB = b3-a3;
        Vec3d aToC = c3-a3;
        Vec3d v = aToB.normalize();
        Vec3d normal = v.cross(aToC.normalize());
        Vec3d w = normal.cross(v);
        triangles[i].vert[0] = {0, 0};
        triangles[i].vert[1] = {aToB.len(), 0};
        triangles[i].vert[2] = {v.dot(aToC), w.dot(aToC)};
        // corresponding edges
        processEdge(ind0, ind1, i, 0);
        processEdge(ind1, ind2, i, 1);
        processEdge(ind2, ind0, i, 2);
    }
}

State GeoGraph::traverse(State state, float dist){
    Triangle myTri = triangles[state.tri];
    auto [untilHit, side] = myTri.rayIntersect(state.pos, state.dir);
    if(dist < untilHit){
        state.pos = state.pos + state.dir * dist;
        return state;
    }
    dist -= untilHit;
    state.tri = myTri.shared[side].first;
    // adjust direction
    Triangle nextTri = triangles[myTri.shared[side].first];
    short nextSide = myTri.shared[side].second;
    bool negate = false;
    if(nextSide >= 3){
        nextSide -= 3;
        negate = true;
    }
    Vec2d myA = myTri.vert[(side+1)%3], myB = myTri.vert[side];
    Vec2d toA = nextTri.vert[(nextSide+1)%3], toB = nextTri.vert[nextSide];
    if(negate) swap(toA, toB);
    Vec2d from = (myA - myB).normalize();
    Vec2d fp = from.perp();
    Vec2d to = (toA - toB).normalize();
    Mat2d mat = Mat2d{{from.x, fp.x}, {from.y, fp.y}} * Mat2d{to, to.perp()};
    state.dir = mat * state.dir;
    // adjust position
    state.pos = toB + to * (from.dot(state.pos));

    return traverse(state, dist);
}

