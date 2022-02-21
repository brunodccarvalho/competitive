#pragma once

#include "algo/y_combinator.hpp"
#include "struct/disjoint_set.hpp"
#include "geometry/geometry2d.hpp"
#include "geometry/wedge.hpp"

template <typename H>
inline auto huge_determinant(H a1, H a2, H a3, H b1, H b2, H b3, H c1, H c2, H c3) {
    return a1 * (b2 * c3 - c2 * b3) - a2 * (b1 * c3 - c1 * b3) + a3 * (b1 * c2 - c1 * b2);
}
inline auto delaunay_determinant(Pt2 p, Pt2 a, Pt2 b, Pt2 c) {
    return huge_determinant<Pt2::H>(a.x - p.x, a.y - p.y, norm2(a) - norm2(p), //
                                    b.x - p.x, b.y - p.y, norm2(b) - norm2(p), //
                                    c.x - p.x, c.y - p.y, norm2(c) - norm2(p));
}
inline bool inside_circumference(Pt2 p, Pt2 a, Pt2 b, Pt2 c) {
    return delaunay_determinant(p, a, b, c) > 0;
}

// Coincident points not supported. Returns a hull edge. O(n log n) decent constant
auto delaunay(const vector<Pt2>& pts) {
    int N = pts.size();
    vector<int> index(N), rank(N);
    iota(begin(index), end(index), 0);
    sort(begin(index), end(index), [&](int i, int j) { return pts[i] < pts[j]; });

    for (int i = 0; i < N; i++) {
        rank[index[i]] = i;
    }

    // Is p inside circle[a,b,c] given ccw? (c->a edge not required)
    auto in_circle = [&](int p, Wedge* edge) {
        int a = edge->vertex, b = edge->target(), c = edge->next->target();
        return inside_circumference(pts[p], pts[a], pts[b], pts[c]);
    };

    auto orient = [&](int a, int b, int c) {
        return orientation(pts[a], pts[b], pts[c]);
    };

    return y_combinator([&](auto self, int l, int r) -> array<Wedge*, 2> {
        if (l + 2 == r) {
            int a = index[l], b = index[l + 1];
            auto A = Wedge::loop(a, b);
            return {A, A->mate};
        } else if (l + 3 == r) {
            int a = index[l], b = index[l + 1], c = index[l + 2];
            if (auto abc = orient(a, b, c); abc > 0) {
                auto A = Wedge::triangle(a, b, c);
                return {A, A->next->mate};
            } else if (abc < 0) {
                auto A = Wedge::triangle(a, c, b);
                return {A, A->mate};
            } else {
                auto A = Wedge::line(a, b, c);
                return {A, A->next->mate};
            }
        }

        int m = (l + r) / 2;
        auto [B, A] = self(l, m);
        auto [D, C] = self(m, r);

        // Let's advance A and retreat D until [AD] is the low base edge
        while (true) {
            if (orient(A->vertex, D->target(), D->vertex) > 0) {
                D = D->rnext();
            } else if (orient(D->vertex, A->vertex, A->target()) > 0) {
                A = A->next;
            } else {
                break;
            }
        }

        // Retriangulate from [A,D] until the top, current base edge is E
        // Keep both A and D pointing 'upwards', away from E
        A = A->rotccw(), D = D->rotcw();
        auto E = Wedge::connect(A->mate, D);
        B = B->vertex == A->vertex ? E : B;
        C = C->vertex == D->vertex ? E->mate : C;

        auto is_valid = [&](auto edge) {
            return edge->next != E && edge->next != E->mate &&
                   orient(E->vertex, E->target(), edge->target()) > 0;
        };

        while (true) {
            while (is_valid(A) && !A->straight_prev() && in_circle(E->target(), A)) {
                A = Wedge::cut_ccw(A);
            }
            while (is_valid(D) && !D->straight_prev() && in_circle(E->vertex, D->mate)) {
                D = Wedge::cut_cw(D);
            }
            if (!is_valid(A) && !is_valid(D)) {
                break;
            } else if (!is_valid(A)) {
                D = D->next;
            } else if (!is_valid(D)) {
                A = A->rnext();
            } else if (in_circle(D->target(), A->mate)) {
                D = D->next;
            } else {
                A = A->rnext();
            }
            E = Wedge::connect(A->mate, D);
        }

        return {B, C};
    })(0, N)[1]; // delaunay does not need to use the temporary pool
}

// Pass in data from delaunay triangulation. O(n log n)
auto euclidean_mst(Wedge* data, const vector<Pt2>& pts) {
    int N = pts.size();
    vector<tuple<Pt2::L, int, int>> edges;
    for (auto [a, b] : Wedge::extract_edges(data)) {
        edges.push_back({dist2(pts[a], pts[b]), a, b});
    }

    double cost = 0;
    disjoint_set dsu(N);
    vector<array<int, 2>> tree;
    sort(begin(edges), end(edges));

    for (auto [dab, a, b] : edges) {
        if (dsu.join(a, b)) {
            cost += std::sqrt(dab);
            tree.push_back({a, b});
        }
    }

    return make_pair(cost, move(tree));
}

auto detriangulate_delaunay(Wedge* hull, const vector<Pt2>& pts) {
    auto bfs = Wedge::linearize(hull);
    int S = bfs.size();

    // Mark the hull so we don't check it nor delete it
    int dead = Wedge::internal_mark++, remove = Wedge::internal_mark++;
    do {
        hull->mark = dead, hull = hull->next;
    } while (hull->mark != dead);

    for (int i = 0; i < S; i++) {
        if (bfs[i]->mark != dead && bfs[i]->mate->mark > i) {
            int a = bfs[i]->vertex, b = bfs[i]->target();
            int c = bfs[i]->next->mate->vertex;
            int d = bfs[i]->mate->prev->vertex;
            if (!inside_circumference(pts[b], pts[c], pts[a], pts[d])) {
                bfs[i]->mark = remove;
            }
        }
    }
    for (int i = 0; i < S; i++) {
        if (bfs[i]->mark == remove) {
            Wedge::cut(bfs[i]);
        }
    }
    return hull;
}
