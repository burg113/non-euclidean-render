
#include <vector>
#include <tuple>
#include <cmath>
#include <map>

using namespace std;

constexpr float eps = 1e-5;

struct Vec3d{
    float x, y, z;

    Vec3d(float x=0, float y=0, float z=0):x(x),y(y),z(z) {}

    Vec3d cross(Vec3d other){
        return {y*other.z - z*other.y,  z*other.x - x*other.z, x*other.y - y*other.x};
    }
    float dot(Vec3d other){
        return x*other.x + y*other.y + z*other.z;
    }

    float lenSq(){
        return x*x + y*y + z*z;
    }

    float len(){
        return sqrt(lenSq());
    }

    Vec3d normalize(){
        return this->operator*(1.0 / len());
    }

    Vec3d operator* (float lambda){
        return {lambda*x, lambda*y, lambda*z};
    }
    Vec3d operator+ (Vec3d other){
        return {x+other.x, y+other.y, z+other.z};
    }
    Vec3d operator- (Vec3d other){
        return {x-other.x, y-other.y, z-other.z};
    }
};

struct Vec2d{
    float x, y;

    Vec2d(float x=0, float y=0):x(x),y(y) {}

    float cross(Vec2d other){
        return x*other.y - y*other.x;
    }
    float dot(Vec2d other){
        return x*other.x + y*other.y;
    }

    Vec2d perp(){
        return {-y, x};
    }

    float lenSq(){
        return x*x + y*y;
    }

    float len(){
        return sqrt(lenSq());
    }

    Vec2d normalize(){
        return this->operator*(1.0f / len());
    }

    Vec2d operator* (float lambda){
        return {lambda*x, lambda*y};
    }
    Vec2d operator+ (Vec2d other){
        return {x+other.x, y+other.y};
    }
    Vec2d operator- (Vec2d other){
        return {x-other.x, y-other.y};
    }
};

struct Mat2d{
    Vec2d v1, v2;

    Mat2d operator* (Mat2d other){
        return {this->operator*(other.v1), this->operator*(other.v2)};
    }

    Vec2d operator* (Vec2d other){
        return v1*other.x + v2*other.y;
    }

};

struct State{
    int tri;
    Vec2d pos, dir;
};

// p, q positions, v, w directions
pair<float, float> lineIntersect(Vec2d p, Vec2d v, Vec2d q, Vec2d w){
    Vec2d d = q-p;
    float div = w.x*v.y - w.y*v.x;
    float s = (w.x*d.y - w.y*d.x) / div;
    float t = (v.x*d.y - v.y*d.x) / div;
    return {s, t};
}

struct Triangle{
    Vec2d vert[3];
    // (triangle_id, orientation)
    // index i means side from i to i+1
    // add 3 if other direction (i+1 -> i)
    pair<int, short> shared[3];

    // (distance, side)
    pair<float, short> rayIntersect(Vec2d pos, Vec2d dir){
        int side = -1;
        float minDist = 1e8;
        for(int i = 0; i < 3; i++){
            float dist = lineIntersect(pos, dir, vert[i], vert[(i+1)%3]-vert[i]).first;
            if(dist > eps && dist < minDist){
                dist = minDist;
                side = i;
            }
        }
        return {minDist, side};
    }
};

struct GeoGraph{
    vector<Triangle> triangles;

    GeoGraph(vector<Vec3d> vertices, vector<tuple<int, int, int>> triangleVerts) {
        int n = (int)vertices.size();
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

    State traverse(State state, float dist){
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

        traverse(state, dist);
    }

};


