
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
        Vec2d normal = (vert[i]-vert[(i+1)%3]).perp().normalized();
        if(normal.dot(dir) > 0 && dist < minDist){
            if(dist < 0){
                dist = 0;
//                throw invalid_argument("Ray-cast started outside of triangle?");
            }
            minDist = dist;
            side = i;
        }
    }
    if(side == -1){
        throw invalid_argument("Ray-cast started outside of triangle?");
    }
    return {minDist, side};
}

Vec2d Triangle::getMid() {
//    return ((vert[0] + vert[1]) * 0.5 + vert[2]) * 0.5;
    return (vert[0] + vert[1] + vert[2]) * (1.0f/3);
}

ostream& operator<<(ostream &stream, Triangle triangle){
    stream << "===============================\n";
    stream << "A: " << triangle.vert[0] << "  B: " << triangle.vert[1] << "  C: " << triangle.vert[2] << "\n";
    stream << "AB = " << (triangle.vert[1]-triangle.vert[0]).len() << "  BC = " <<
    (triangle.vert[2]-triangle.vert[1]).len() << "  CA = " << (triangle.vert[0]-triangle.vert[2]).len() << "\n";
    return stream;
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
            triangles[tri].nextTriangle[side] = tri2;
            triangles[tri].nextSide[side] = side2;
            triangles[tri2].nextTriangle[side2] = tri;
            triangles[tri2].nextSide[side2] = side;
        }
        else if(byEdge.count((p2))){
            auto [tri2, side2] = byEdge[p2];
            triangles[tri].nextTriangle[side] = tri2;
            triangles[tri].nextSide[side] = side2 + 3;
            triangles[tri2].nextTriangle[side2] = tri;
            triangles[tri2].nextSide[side2] = side + 3;
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
        Vec3d v = aToB.normalized();
        Vec3d normal = v.cross(aToC.normalized()).normalized();
        Vec3d w = normal.cross(v).normalized();
        triangles[i].vert[0] = {0, 0};
        triangles[i].vert[1] = {aToB.len(), 0};
        triangles[i].vert[2] = {v.dot(aToC), w.dot(aToC)};
        triangles[i].normal3d = normal;
        // corresponding edges
        processEdge(ind0, ind1, i, 0);
        processEdge(ind1, ind2, i, 1);
        processEdge(ind2, ind0, i, 2);
    }

    // precompute transition data
    for(Triangle &triangle : triangles){
        for(int side = 0; side < 3; side++){
            Triangle nextTri = triangles[triangle.nextTriangle[side]];
            int nextSide = triangle.nextSide[side];
            bool negate = false;
            if(nextSide >= 3){
                nextSide -= 3;
                negate = true;
            }
            Vec2d myA = triangle.vert[(side+1)%3], myB = triangle.vert[side];
            Vec2d toA = nextTri.vert[(nextSide+1)%3], toB = nextTri.vert[nextSide];
            if(negate) swap(toA, toB);
            Vec2d my = (myA - myB).normalized();
            Vec2d myPerp = my.perp();
            Vec2d to = (toA - toB).normalized();
            Mat2d mat = Mat2d{to, to.perp()} * Mat2d{{my.x, myPerp.x}, {my.y, myPerp.y}};

            triangle.rotationMatrix[side] = mat;
            triangle.nextVert[side][0] = toB;
            triangle.nextVert[side][1] = toA;
        }
    }
}

pair<State, float> GeoGraph::traverse(State state, float dist){
    while(true){
        if (dist == 0)
            return {state, 0};

        Triangle myTri = triangles[state.tri];
        auto [untilHit, side] = myTri.rayIntersect(state.pos, state.dir);
        if(dist < untilHit){
            state.pos += state.dir * dist;
            return {state, untilHit-dist};
        }
        dist -= untilHit;
        state.pos = state.pos + state.dir*untilHit;
        state.tri = myTri.nextTriangle[side];
        state.dir = myTri.rotationMatrix[side] * state.dir;
        float sidePosition = (myTri.vert[(side+1)%3] - myTri.vert[side]).normalized().dot(state.pos - myTri.vert[side]);
        state.pos = myTri.nextVert[side][0] + (myTri.nextVert[side][1] - myTri.nextVert[side][0]).normalized() * sidePosition;
    }
}


