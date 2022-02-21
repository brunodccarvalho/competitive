#pragma once

#include "geometry/geometry3d.hpp"
#include "geometry/wedge.hpp"

// Coincident points supported. Returns a hull edge (1d/2d/3d). O(n log n)
// Supports double coords. 200ms-4s for 500K points depending on distribution. Returns a
// strict hull if 1d/2d or 3d with integers, else a 3d triangulated hull with doubles
struct quickhull {
    using FaceQueue = set<pair<double, int>, greater<pair<double, int>>>;
    struct FaceData {
        Wedge* edge;
        int eye = -1;
        double furthest = 0;
        Pt3 normal;
        FaceQueue::iterator iterator;
        explicit FaceData(Wedge* edge, const Pt3& normal) : edge(edge), normal(normal) {}
    };

    int N;
    const vector<Pt3>& pts;
    vector<FaceData> data;
    vector<int> eye_next, eye_prev, state, freelist;
    FaceQueue fq;
    static constexpr int DEAD = -1, PLAIN = 0, MERGE = 1;

    explicit quickhull(const vector<Pt3>& pts)
        : N(pts.size()), pts(pts), eye_next(N + 1), eye_prev(N + 1) {}

    auto icross(int a, int b, int c) const { return cross(pts[a], pts[b], pts[c]); }

    auto turnup(int eye, Wedge* u) const { return icross(eye, u->vertex, u->target()); }

    auto turnup(Wedge* u) const {
        return icross(u->vertex, u->target(), u->next->target());
    }

    auto iline(int a, int b, int c) const { return collinear(pts[a], pts[b], pts[c]); }

    auto edist(int p, Wedge* u) const {
        return signed_planedist(pts[p], pts[u->vertex], data[u->mark].normal);
    }

    auto orient(int a, int b, int c, int p) const {
        return orientation(pts[a], pts[b], pts[c], pts[p]);
    }

    auto orient(int p, Wedge* u) const {
        int a = u->vertex, b = u->target(), c = u->next->target();
        return orient(a, b, c, p);
    }

    // --- Face and eye management routines

    void link_eye(int u, int v) { eye_next[u] = v, eye_prev[v] = u; }
    void link_eye(int u, int v, int w) { link_eye(u, v), link_eye(v, w); }

    int new_face(Wedge* edge, const Pt3& normal = Pt3()) {
        if (freelist.empty()) {
            int face = data.size();
            data.emplace_back(edge, normal);
            state.push_back(0);
            return face;
        } else {
            int face = freelist.back();
            freelist.pop_back();
            data[face] = FaceData(edge, normal);
            state[face] = 0;
            return face;
        }
    }

    void add_eye(int eye, int face, double dist) {
        if (data[face].eye == -1) { // first eye for this face
            data[face].eye = eye_next[eye] = eye_prev[eye] = eye;
            data[face].iterator = fq.emplace(dist, face).first;
            data[face].furthest = dist;
        } else if (data[face].furthest < dist) { // new furthest eye for this face
            link_eye(data[face].eye, eye, eye_next[data[face].eye]);
            data[face].eye = eye;
            fq.erase(data[face].iterator);
            data[face].iterator = fq.emplace(dist, face).first;
            data[face].furthest = dist;
        } else { // new eye for this face, but not furthest
            link_eye(data[face].eye, eye, eye_next[data[face].eye]);
        }
    }

    // --- Main routines

    auto add_vertex_to_hull(int eye, int eye_face) {
        vector<int> shadowed_faces;
        vector<Wedge*> bfs, shadowed_edges;
        Wedge* beach;

        // Mark and cut strictly shadowed face with bfs and two-side shadowed edges
        auto eliminate_face = [&](int face) {
            state[face] = DEAD;
            shadowed_faces.push_back(face);
            Wedge* edge = data[face].edge;
            do {
                if (state[edge->mate->mark] == DEAD) {
                    shadowed_edges.push_back(edge);
                }
                bfs.push_back(edge), edge = edge->next;
            } while (edge != data[face].edge);
        };
        eliminate_face(eye_face);

        // Eliminate all shadowed faces, collect shadowed edges and get a horizon edge ccw
        for (int i = 0, S = bfs.size(); i < S; i++, S = bfs.size()) {
            int face = bfs[i]->mate->mark;
            if (state[face] != DEAD && orient(eye, bfs[i]->mate) == +1) {
                eliminate_face(face);
            } else if (state[face] != DEAD) {
                beach = bfs[i];
            }
        }
        assert(state[beach->mate->mark] != DEAD);

        for (auto edge : shadowed_edges) {
            Wedge::cut(edge);
        }

        // Transfer eyes from shadowed faces to an open list, and append to faces freelist
        eye_next[N] = eye_prev[N] = N;
        for (int face : shadowed_faces) {
            if (data[face].eye != -1) {
                int u = data[face].eye, v = eye_prev[u];
                link_eye(v, eye_next[N]), link_eye(N, u);
                fq.erase(data[face].iterator);
            }
            freelist.push_back(face);
        }

        // Compute the horizon points. For float coordinates, triangulate the entire
        // horizon and do not merge faces. For integer coordinates merge faces and skip
        // support edges that result in coplanar adjacent support faces
        vector<Wedge*> horizon;

        if constexpr (Pt3::FLOAT) {
            Wedge* first = beach;
            do {
                beach->mark = PLAIN;
                horizon.push_back(beach);
                beach = beach->next;
            } while (beach != first);
        } else {
            Wedge* first = beach;
            do {
                if (orient(eye, beach->prev) == +1) {
                    beach->mark = orient(eye, beach->mate) == 0 ? MERGE : PLAIN;
                    horizon.push_back(beach);
                }
                beach = beach->next;
            } while (beach != first);
        }

        int F = horizon.size();
        assert(F >= 3);

        // Link adjoining edges to horizon, use mate's face if merging coplanar faces
        vector<Wedge*> support(F);
        for (int f = 0; f < F; f++) {
            support[f] = Wedge::connecto(eye, horizon[f]);
        }

        // Connect the support edges
        for (int f = 0; f < F; f++) {
            int g = f + 1 < F ? f + 1 : 0;
            Wedge::link(support[g]->mate, support[f]);

            Wedge* link = support[f];
            Wedge* start = support[f]->next;
            Wedge* finish = support[g]->mate;
            int oface = start->mate->mark;

            if (Pt3::FLOAT || start->mark == PLAIN) {
                int face = finish->mark = link->mark = new_face(link);

                do {
                    data[face].normal += turnup(eye, start);
                    start->mark = face, start = start->next;
                } while (start != finish);
            } else {
                int face = finish->mark = link->mark = oface;
                data[face].edge = link;

                do {
                    data[face].normal += turnup(eye, start);
                    start = start->next, Wedge::cut(start->prev);
                } while (start != finish);
            }
        }

        // Adjacent coplanar merges lead to straight edges that need to be melded
        for (int f = 0; f < F; f++) {
            if (support[f]->straight_next()) {
                Wedge::meld_with_prev(support[f]->next);
            }
        }

        // We're done with the merge proper, now resolve open points' conflicts
        for (int v = eye_next[N], u = eye_next[v]; v != N; v = u, u = eye_next[v]) {
            if (v == eye) {
                continue;
            }
            double maxdist = 0;
            int maxface = -1;
            for (Wedge* face : support) {
                if (!Pt3::FLOAT || orient(v, face) == +1) {
                    auto dist = edist(v, face);
                    if (maxdist < dist) {
                        maxdist = dist, maxface = face->mark;
                    }
                }
            }
            if (maxface != -1) {
                add_eye(v, maxface, maxdist);
            }
        }

        return support[0]; // any valid hull edge
    }

    auto hull3d(int v0, int v1, int v2, int v3) {
        // Create initial tetrahedron
        if (orient(v0, v1, v2, v3) == -1) {
            swap(v1, v2);
        }
        auto hull = Wedge::tetrahedron(v0, v1, v2, v3);
        for (Wedge* edge : {hull, hull->mate, hull->rnext(), hull->rprev()}) {
            int face = new_face(edge, turnup(edge));
            edge->mark = edge->next->mark = edge->prev->mark = face;
        }

        // Populate eyes (conflicts)
        for (int v = 0; v < N; v++) {
            if (v != v0 && v != v1 && v != v2 && v != v3) {
                double maxdist = 0;
                int maxface = -1;
                for (const auto& face : data) {
                    if (!Pt3::FLOAT || orient(v, face.edge) == +1) {
                        auto dist = edist(v, face.edge);
                        if (maxdist < dist) {
                            maxdist = dist, maxface = face.edge->mark;
                        }
                    }
                }
                if (maxface != -1) {
                    add_eye(v, maxface, maxdist);
                }
            }
        }

        // Augment the hull while there are still points (eyes) outside of it
        while (!fq.empty()) {
            int face = fq.begin()->second;
            hull = add_vertex_to_hull(data[face].eye, face);
        }

        return hull;
    }

    auto hull2d(int v0, int v1, int v2) { // Graham scan in 3D
        int i = 0, S = 0, x = min_element(begin(pts), end(pts)) - begin(pts);
        auto up = icross(v0, v1, v2);

        vector<int> index(N), hull;
        iota(begin(index), end(index), 0);
        sort(begin(index), end(index), [&](int u, int v) {
            if (auto xuv = dot(up, icross(x, u, v)); xuv != 0) {
                return xuv > 0;
            } else {
                return dist2(pts[x], pts[u]) < dist2(pts[x], pts[v]);
            }
        });

        while (i < N) {
            while (S > 1 && dot(up, icross(hull[S - 2], hull[S - 1], index[i])) <= 0) {
                hull.pop_back(), S--;
            }
            hull.push_back(index[i++]), S++;
        }

        return Wedge::polygon(hull);
    }

    auto hull1d(int v0, int v1) { return Wedge::loop(v0, v1); }

    Wedge* solve() {
        // Compute extreme points along each main axis, breaking ties lexicographically
        int minpt[3] = {0, 0, 0}, maxpt[3] = {0, 0, 0};

        for (int v = 1; v < N; v++) {
            for (int d = 0; d < 3; d++) {
                if (pts[minpt[d]][d] > pts[v][d]) {
                    minpt[d] = v;
                } else if (pts[minpt[d]][d] == pts[v][d]) {
                    minpt[d] = pts[v] < pts[minpt[d]] ? v : minpt[d];
                }
                if (pts[maxpt[d]][d] < pts[v][d]) {
                    maxpt[d] = v;
                } else if (pts[maxpt[d]][d] == pts[v][d]) {
                    maxpt[d] = pts[v] > pts[maxpt[d]] ? v : maxpt[d];
                }
            }
        }

        int v0 = -1, v1 = -1, v2 = -1, v3 = -1;
        Pt3::L maxdist = 0;

        // Select v0, v1 such that dist(v0,v1) is largest
        for (int d = 0; d < 3; d++) {
            auto gap = pts[maxpt[d]][d] - pts[minpt[d]][d];
            if (maxdist < gap) {
                maxdist = gap, v0 = minpt[d], v1 = maxpt[d];
            }
        }
        if (v0 == -1 || v1 == -1) {
            return nullptr;
        }

        // Select v2 such that linedist(v2, v0, v1) is largest
        maxdist = 0;
        for (int v = 0; v < N; v++) {
            if (v != v0 && v != v1) {
                if (!Pt3::FLOAT || !iline(v0, v1, v)) {
                    auto dist = norm2(icross(v, v0, v1));
                    if (maxdist < dist) {
                        maxdist = dist, v2 = v;
                    }
                }
            }
        }
        if (v2 == -1) {
            return hull1d(v0, v1);
        }

        // Select v3 such that unsigned planedist(v3,v0,v1,v2) is maximum (furthest)
        auto normal = icross(v0, v1, v2);
        maxdist = 0;
        for (int v = 0; v < N; v++) {
            if (v != v0 && v != v1 && v != v2) {
                if (!Pt3::FLOAT || orient(v0, v1, v2, v) != 0) {
                    auto dist = abs(dot(pts[v] - pts[v0], normal));
                    if (maxdist < dist) {
                        maxdist = dist, v3 = v;
                    }
                }
            }
        }
        if (v3 == -1) {
            return hull2d(v0, v1, v2);
        } else {
            return hull3d(v0, v1, v2, v3);
        }
    }

    static auto compute(const vector<Pt3>& pts) {
        quickhull solver(pts);
        return solver.solve(); // quickhull doesn't need to use the temporary pool
    }
};
