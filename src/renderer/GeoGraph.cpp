
#include <vector>
#include <tuple>
#include <map>

#include "../util/MathUtil.h"
#include "glm/gtx/norm.hpp"
#include "GeoGraph.h"

using namespace std;
using glm::mat2, glm::vec2, glm::vec3, glm::normalize, glm::dot, glm::cross, glm::length, glm::length2;

pair<float, short> Triangle::rayIntersect(vec2 pos, vec2 dir){
    int side = -1;
    float minDist = 1e8;
    for(int i = 0; i < 3; i++){
        float dist = lineIntersect(pos, dir, vert[i], vert[(i+1)%3]-vert[i]).first;
        vec2 normal = normalize(perp(vert[i]-vert[(i+1)%3]));
        if(dot(normal, dir) > 0 && dist < minDist){
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

vec2 Triangle::getMid() {
//    return ((vert[0] + vert[1]) * 0.5 + vert[2]) * 0.5;
    return (vert[0] + vert[1] + vert[2]) / 3.0f;
}

void Triangle::from3d(vec3 a, vec3 b, vec3 c, vec2 uvA, vec2 uvB, vec2 uvC) {
    vec3 aToB = b - a;
    vec3 aToC = c - a;
    vec3 right = normalize(aToB);
    this->normal3d = normalize(cross(right, normalize(aToC)));
    vec3 up = normalize(cross(normal3d, right));
    this->vert[0] = vec2(0, 0);
    this->vert[1] = vec2(length(aToB), 0);
    this->vert[2] = vec2(dot(right, aToC), dot(up, aToC));
    vec2 uvAtoB = uvB-uvA;
    vec2 uvAtoC = uvC-uvA;
    this->uv0 = uvA;
    this->uvRight = uvAtoB / vert[1].x;
    this->uvUp = (uvAtoC - uvAtoB * dot(uvAtoC, uvAtoB) / length2(uvAtoB)) / vert[2].y;
}

vec2 Triangle::getUV(vec2 pos) const {
    return uv0 + uvRight * pos.x + uvUp * pos.y;
}

ostream& operator<<(ostream &stream, Triangle triangle){
    stream << "===============================\n";
    stream << "A: " << triangle.vert[0] << "  B: " << triangle.vert[1] << "  C: " << triangle.vert[2] << "\n";
    stream << "AB = " << length(triangle.vert[1]-triangle.vert[0]) << "  BC = " <<
    length(triangle.vert[2]-triangle.vert[1]) << "  CA = " << length(triangle.vert[0]-triangle.vert[2]) << "\n";
    return stream;
}

GeoGraph::GeoGraph(vector<vec3> vertices, vector<tuple<int, int, int>> triangleVerts, vector<vec2> uv, vector<tuple<int, int, int>> triangleUV) {
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
        auto [uvInd0, uvInd1, uvInd2] = triangleUV[i];
        triangles[i].from3d(vertices[ind0], vertices[ind1], vertices[ind2], uv[uvInd0], uv[uvInd1], uv[uvInd2]);
        // corresponding edges
        processEdge(ind0, ind1, i, 0);
        processEdge(ind1, ind2, i, 1);
        processEdge(ind2, ind0, i, 2);
    }

    // precompute transition data
    for(Triangle &triangle : triangles){
        for(int side = 0; side < 3; side++){
            if (triangle.nextTriangle[side] == -1) {
                continue;
            }
            Triangle nextTri = triangles[triangle.nextTriangle[side]];

            int nextSide = triangle.nextSide[side];
            bool negate = false;
            if(nextSide >= 3){
                nextSide -= 3;
                negate = true;
            }
            vec2 myA = triangle.vert[(side+1)%3], myB = triangle.vert[side];
            vec2 toA = nextTri.vert[(nextSide+1)%3], toB = nextTri.vert[nextSide];
            if(negate) swap(toA, toB);
            vec2 my = normalize(myA - myB);
            vec2 myPerp = perp(my);
            vec2 to = normalize(toA - toB);
            mat2 mat = mat2(to, perp(to)) * mat2(my.x, myPerp.x, my.y, myPerp.y);

            triangle.rotationMatrix[side] = mat;
            triangle.nextVert[side][0] = toB;
            triangle.nextVert[side][1] = toA;
        }
    }
}

// returns state with tri = -1 if end of mesh was reached
pair<State, float> GeoGraph::traverse(State state, float dist){
    if (state.tri == -1)
        return {state, 0};
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
        // hitting boarder of mesh
        if (state.tri == -1) {
            return {state, 0};
        }

        state.dir = myTri.rotationMatrix[side] * state.dir;
        float sidePosition = dot(normalize(myTri.vert[(side+1)%3] - myTri.vert[side]), state.pos - myTri.vert[side]);
        state.pos = myTri.nextVert[side][0] + normalize(myTri.nextVert[side][1] - myTri.nextVert[side][0]) * sidePosition;
    }
}


