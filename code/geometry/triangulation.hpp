#pragma once

#include "geometry/geometry2d.hpp"
#include "geometry/wedge.hpp"
#include "geometry/shaft_scanner.hpp"
#include "algo/y_combinator.hpp"

// Greedy sweepline triangulation along x. Returns a hull edge. O(n) after sort
auto sweep_triangulation(const vector<Pt2>& pts) {
    int N = pts.size();
    vector<int> index(N);
    iota(begin(index), end(index), 0);
    sort(begin(index), end(index), [&](int i, int j) { return pts[i] < pts[j]; });

    Wedge* hull = Wedge::loop(index[1], index[0]);

    auto can_see_edge = [&](int u, Wedge* edge) {
        return orientation(pts[u], pts[edge->vertex], pts[edge->target()]) > 0;
    };

    for (int i = 2; i < N; i++) {
        int v = index[i];
        hull = Wedge::connecto(v, hull);
        Wedge* left = hull->next;
        Wedge* right = hull->mate->prev;
        while (can_see_edge(v, left)) {
            Wedge::connect(hull->mate, left = left->next);
        }
        hull = left->prev;
        while (can_see_edge(v, right)) {
            Wedge::connect(right = right->prev, hull);
        }
    }

    return hull;
}

// Merge two strictly disjoint convex triangulations given any two hull edges. O(|L|+|R|)
auto merge_triangulations(Wedge* L, Wedge* R, const vector<Pt2>& pts) {
    auto icross = [&](int a, int b, int c) { return cross(pts[a], pts[b], pts[c]); };

    // Walk through L and R hulls CW and find the minimum vertex on each
    Wedge *A = L, *B = L->next, *C = R->next, *D = R;
    while (B != L) {
        if (pts[B->vertex] < pts[A->vertex]) {
            A = B;
        }
        B = B->next;
    }
    while (C != R) {
        if (pts[C->vertex] < pts[D->vertex]) {
            D = C;
        }
        C = C->next;
    }
    if (pts[D->vertex] < pts[A->vertex]) {
        swap(A, D);
    }

    // Find the higher supporting bridge between A and D
    while (true) {
        while (icross(A->vertex, D->vertex, D->target()) > 0) {
            D = D->next;
        }
        if (icross(A->vertex, D->vertex, A->target()) > 0) {
            A = A->next;
        } else {
            break;
        }
    }

    // Rotate D inwards and swap A,D so that the low bridge has A on the left
    D = D->rotccw(), swap(A, D);
    auto E = Wedge::connect(A->mate, D);

    while (true) {
        auto a = icross(A->vertex, D->vertex, A->target());
        auto d = icross(A->vertex, D->vertex, D->target());
        if (a <= 0 && d <= 0) {
            break;
        } else if (a <= 0) {
            D = D->next;
        } else if (d <= 0) {
            A = A->rnext();
        } else if (a > d) { // A->target can't lie inside [A->vertex D->vertex D->target]
            D = D->next;
        } else {
            A = A->rnext();
        }
        E = Wedge::connect(A->mate, D);
    }

    return E;
}

// Build a triangulation from a list of segments. O(n log n)
// lower[] first edge going right in ccw order. upper[] first edge going left in ccw order
auto build_triangulation(const vector<Pt2>& pts, vector<array<int, 2>> segments) {
    int N = pts.size(), S = segments.size();
    for (auto& [u, v] : segments) {
        if (pts[v] < pts[u]) {
            swap(u, v);
        }
    }

    vector<int> order(S);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order), [&](int i, int j) {
        auto [a, b] = segments[i];
        auto [c, d] = segments[j];
        return cross(pts[a] - pts[b], pts[c] - pts[d]) > 0;
    });

    vector<Wedge*> upper(N), lower(N), skeleton(S);
    for (int s : order) {
        auto [u, v] = segments[s];
        Wedge* a = upper[u] ? upper[u]->mate : lower[u] ? lower[u]->mate : nullptr;
        Wedge* b = lower[v] ? lower[v]->rotcw() : upper[v] ? upper[v]->rotcw() : nullptr;
        Wedge* link = Wedge::safe_connect(u, v, a, b);
        lower[u] = lower[u] ? lower[u] : link;
        upper[v] = upper[v] ? upper[v] : link->mate;
        skeleton[s] = u == segments[s][0] ? link : link->mate;
        link->data = link->mate->data = s;
    }

    return make_tuple(move(lower), move(upper), move(skeleton));
}

// Sweep triangulation with some fixed, non-intersecting segments. O(n log n)
auto constrained_triangulation(const vector<Pt2>& pts, vector<array<int, 2>> segments,
                               int add_mark = -2) {
    int N = pts.size(), S = segments.size();
    vector<int> index(N), rank(N);
    iota(begin(index), end(index), 0);
    sort(begin(index), end(index), [&](int u, int v) { return pts[u] < pts[v]; });

    for (int i = 0; i < N; i++) {
        rank[index[i]] = i;
    }

    vector<int8_t> forward(S);
    for (int s = 0; s < S; s++) {
        auto& [u, v] = segments[s];
        if (rank[u] < rank[v]) {
            forward[s] = true;
        } else {
            forward[s] = false;
            swap(u, v);
            assert(u != v);
        }
    }

    // Sort the segments radially. Ties do not need to be broken if input is valid
    vector<int> order(S);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order), [&](int i, int j) {
        auto [a, b] = segments[i];
        auto [c, d] = segments[j];
        return cross(pts[a] - pts[b], pts[c] - pts[d]) > 0;
    });

    // Setup the initial disconnected triangulation mesh
    // Edges on the left / edges on the right of any vertex u are added in CCW order
    vector<Wedge*> upper(N), lower(N), skeleton(S);
    for (int s : order) {
        auto [u, v] = segments[s];
        Wedge* a = upper[u] ? upper[u]->mate : lower[u] ? lower[u]->mate : nullptr;
        Wedge* b = lower[v] ? lower[v]->rotcw() : upper[v] ? upper[v]->rotcw() : nullptr;
        Wedge* link = Wedge::safe_connect(u, v, a, b);
        lower[u] = lower[u] ? lower[u] : link;
        upper[v] = upper[v] ? upper[v] : link->mate;
        skeleton[s] = link;
        link->mark = link->mate->mark = s;
    }

    // Shafts map, for vertices not assigned to any constraint
    map<int, Wedge*, shaft_scanner> shafts(shaft_scanner(pts, rank, segments));
    Wedge* hull = nullptr;

    auto can_see_edge = [&](int u, Wedge* edge) {
        int v = edge->vertex, w = edge->target();
        return rank[v] < rank[u] && rank[w] < rank[u] &&
               orientation(pts[u], pts[v], pts[w]) > 0;
    };

    // Insert shafts for leftmost vertex
    if (int u = index[0]; lower[u]) {
        Wedge* front = lower[u];
        auto shaft = shafts.begin();
        do {
            shaft = shafts.emplace_hint(shaft, front->mark, front);
            front = front->rotccw();
        } while (front != lower[u]);

        // For the INF shaft, the hull might be engulfed later, so push one edge back
        hull = front->mate;
    }

    for (int i = 1; i < N; i++) {
        int v = index[i];

        if (upper[v]) {
            // Erase shafts ending at this node
            Wedge* back = upper[v];
            Wedge* last = lower[v] ? lower[v] : upper[v];
            do {
                shafts.erase(back->mark);
                back = back->rotccw();
            } while (back != last);

            // Triangulate between the back edges if v has any
            Wedge* sweep = upper[v];
            last = last->rotcw();

            while (sweep != last) {
                Wedge* stop = sweep->prev;
                while (sweep->next->next != stop) {
                    sweep = Wedge::connect_next(sweep);
                    sweep->mark = sweep->mate->mark = add_mark;
                }
                sweep = sweep->rotccw();
            }
        }

        auto shaft = shafts.lower_bound(pts[v]);
        int shaftid = shaft != shafts.end() ? shaft->first : -1;

        // If there is no back edge from v rely on shaft->second to get one
        if (!upper[v]) {
            Wedge* visible = shaftid == -1 ? hull : shaft->second;
            visible = shaftid == -1 && visible ? visible->next : visible;
            Wedge* before = lower[v] ? lower[v]->mate : nullptr;
            upper[v] = Wedge::safe_connect(v, index[0], before, visible);
            upper[v]->mark = upper[v]->mate->mark = add_mark;
        }

        // Triangulate down from v, to the left as seen from v
        Wedge* low = lower[v] ? lower[v]->rotcw() : upper[v]->rotcw();
        while (can_see_edge(v, low->next)) {
            low = Wedge::connect_next(low);
            low->mark = low->mate->mark = add_mark;
        }

        // Triangulate up from v, to the right as seen from v
        Wedge* high = upper[v]->mate;
        while (can_see_edge(v, high->prev)) {
            high = Wedge::connect_prev(high);
            high->mark = high->mate->mark = add_mark;
        }
        upper[v] = high->mate;

        // For the INF shaft, the hull might be engulfed later, so push one edge back
        shaftid == -1 ? hull = low->prev : shaft->second = low;

        // Add shafts opened at this node
        if (lower[v]) {
            Wedge* front = lower[v];
            Wedge* last = upper[v] ? upper[v] : lower[v];
            do {
                shaft = shafts.emplace_hint(shaft, front->mark, front);
                front = front->rotccw();
            } while (front != last);
        }
    }

    return hull; // just a hull edge
    // return make_pair(hull, move(upper), move(skeleton));
}

// Triangulate on the ccw side of a set of non-intersecting segments. O(n log n)
// To recover the triangles proper, ignore all edges with mark ban_mark (side thereof)
// The segments should form a set of closed oriented polygonal lines
auto sided_triangulation(const vector<Pt2>& pts, vector<array<int, 2>> segments,
                         int ban_mark = -1, int add_mark = -2) {
    int N = pts.size(), S = segments.size();
    vector<int> index(N), rank(N);
    iota(begin(index), end(index), 0);
    sort(begin(index), end(index), [&](int u, int v) { return pts[u] < pts[v]; });
    for (int u = 0; u < N; u++) {
        rank[index[u]] = u;
    }

    vector<int8_t> forward(S);
    for (int s = 0; s < S; s++) {
        auto& [u, v] = segments[s];
        if (rank[u] < rank[v]) {
            forward[s] = true;
        } else {
            forward[s] = false;
            swap(u, v);
            assert(u != v);
        }
    }

    vector<int> order(S);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order), [&](int i, int j) {
        auto [a, b] = segments[i];
        auto [c, d] = segments[j];
        return cross(pts[a] - pts[b], pts[c] - pts[d]) > 0;
    });

    // Setup the initial disconnected triangulation mesh
    // Edges on the left / edges on the right of any vertex u are added in CCW order
    vector<Wedge*> upper(N), lower(N), skeleton(S);
    for (int s : order) {
        auto [u, v] = segments[s];
        Wedge* a = upper[u] ? upper[u]->mate : lower[u] ? lower[u]->mate : nullptr;
        Wedge* b = lower[v] ? lower[v]->rotcw() : upper[v] ? upper[v]->rotcw() : nullptr;
        Wedge* link = Wedge::safe_connect(u, v, a, b);
        lower[u] = lower[u] ? lower[u] : link;
        upper[v] = upper[v] ? upper[v] : link->mate;
        skeleton[s] = link;
        link->mark = link->mate->mark = s;
    }

    // Shafts map, for vertices not assigned to any constraint
    map<int, Wedge*, shaft_scanner> shafts(shaft_scanner(pts, rank, segments));
    Wedge* hull = nullptr;

    auto can_see_edge = [&](int u, Wedge* edge) {
        int v = edge->vertex, w = edge->target();
        return rank[v] < rank[u] && rank[w] < rank[u] &&
               orientation(pts[u], pts[v], pts[w]) > 0;
    };
    auto can_triangulate = [&](Wedge* edge) {
        bool right = rank[edge->target()] > rank[edge->vertex];
        return edge->mark == add_mark ||
               (right ? forward[edge->mark] : !forward[edge->mark]);
    };

    // Insert shafts for leftmost vertex
    if (int u = index[0]; lower[u]) {
        Wedge* front = lower[u];
        auto shaft = shafts.begin();
        do {
            shaft = shafts.emplace_hint(shaft, front->mark, front);
            front = front->rotccw();
        } while (front != lower[u]);

        // For the INF shaft, the hull might be engulfed later, so push one edge back
        hull = front->mate;
    }

    for (int i = 1; i < N; i++) {
        int v = index[i];

        // Erase shafts ending at this node, do so before triangulating between them
        if (upper[v]) {
            Wedge* back = upper[v];
            Wedge* last = lower[v] ? lower[v] : upper[v];
            do {
                shafts.erase(back->mark);
                back = back->rotccw();
            } while (back != last);

            // Triangulate between the back edges if v has any, only between backward ones
            Wedge* sweep = upper[v];
            last = last->rotcw();

            while (sweep != last) {
                if (forward[sweep->mark]) {
                    sweep = sweep->rotccw();
                } else {
                    Wedge* stop = sweep->prev;
                    while (sweep->next->next != stop) {
                        sweep = Wedge::connect_next(sweep);
                        sweep->mark = sweep->mate->mark = add_mark;
                    }
                    sweep = sweep->rotccw();
                }
            }
        }

        auto shaft = shafts.lower_bound(pts[v]);
        int shaftid = shaft != shafts.end() ? shaft->first : -1;

        // If there is no back edge from v rely on shaft->second to get one
        if (!upper[v]) {
            Wedge* visible = shaftid == -1 ? hull : shaft->second;
            visible = shaftid == -1 && visible ? visible->next : visible;
            if (!visible || can_triangulate(visible)) {
                Wedge* before = lower[v] ? lower[v]->mate : nullptr;
                upper[v] = Wedge::safe_connect(v, index[0], before, visible);
                upper[v]->mark = upper[v]->mate->mark = add_mark;
            }
        }

        if (upper[v]) {
            // Triangulate down from v, to the left as seen from v
            Wedge* low = lower[v] ? lower[v]->rotcw() : upper[v]->rotcw();
            if (can_triangulate(low)) {
                while (can_triangulate(low->next) && can_see_edge(v, low->next)) {
                    low = Wedge::connect_next(low);
                    low->mark = low->mate->mark = add_mark;
                }
            }

            // Triangulate up from v, to the right as seen from v
            Wedge* high = upper[v]->mate;
            if (can_triangulate(high)) {
                while (can_triangulate(high->prev) && can_see_edge(v, high->prev)) {
                    high = Wedge::connect_prev(high);
                    high->mark = high->mate->mark = add_mark;
                }
                upper[v] = high->mate;
            }

            // For the INF shaft, the hull might be engulfed later, so push one edge back
            shaftid == -1 ? hull = low->prev : shaft->second = low;
        }

        // Add shafts opened at this node
        if (lower[v]) {
            Wedge* front = lower[v];
            Wedge* last = upper[v] ? upper[v] : lower[v];
            do {
                shaft = shafts.emplace_hint(shaft, front->mark, front);
                front = front->rotccw();
            } while (front != last);
        }
    }

    // Tag the outside of constraint segments with mark ban_mark
    for (int s = 0; s < S; s++) {
        if (forward[s]) {
            skeleton[s]->mate->mark = ban_mark;
        } else {
            skeleton[s]->mark = ban_mark;
        }
    }

    return hull; // return just a hull edge
    // return make_tuple(hull, move(upper), move(skeleton));
}

auto polygon_triangulation(const vector<Pt2>& pts, const vector<int>& polygon,
                           int ban_mark = -1, int add_mark = -2) {
    int Pt2 = polygon.size();
    vector<array<int, 2>> segments(Pt2);
    for (int i = 0; i < Pt2; i++) {
        int u = polygon[i], v = polygon[i + 1 < Pt2 ? i + 1 : 0];
        segments[i] = {u, v};
    }
    return sided_triangulation(pts, segments, ban_mark, add_mark);
}

auto multi_polygon_triangulation(const vector<Pt2>& pts, const vector<vector<int>>& polys,
                                 int ban_mark = -1, int add_mark = -2) {
    vector<array<int, 2>> segments;
    for (const auto& polygon : polys) {
        int Pt2 = polygon.size();
        for (int i = 0; i < Pt2; i++) {
            int u = polygon[i], v = polygon[i + 1 < Pt2 ? i + 1 : 0];
            segments.push_back({u, v});
        }
    }
    return sided_triangulation(pts, segments, ban_mark, add_mark);
}
