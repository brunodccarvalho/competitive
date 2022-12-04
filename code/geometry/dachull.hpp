#pragma once

#include "geometry/geometry3d.hpp"
#include "geometry/wedge.hpp"

// Coincident/collinear/coplanar supported. Returns a hull edge (1d/2d/3d). O(n log n)
// Only for integer coords. Computes a strict hull. ~800ms for 500K points.
struct dachull {
    const vector<Pt3>& pts;
    vector<int> index, rank;
    mutable int run = 0;

    explicit dachull(const vector<Pt3>& pts)
        : pts(pts), index(pts.size()), rank(pts.size()) {
        iota(begin(index), end(index), 0);
        sort(begin(index), end(index), [&](int i, int j) { return pts[i] < pts[j]; });
        // Remove coincident points
        int S = 1;
        for (int i = 1, N = index.size(); i < N; i++) {
            if (pts[index[i - 1]] < pts[index[i]]) {
                index[S++] = index[i];
            }
        }
        index.resize(S);
        for (int i = 0; i < S; i++) {
            rank[index[i]] = i;
        }
    }

    auto icross(int a, int b, int c) const { return cross(pts[a], pts[b], pts[c]); }

    auto orient(int a, int b, int c, int d) const {
        return orientation(pts[a], pts[b], pts[c], pts[d]);
    }

    auto vec(Wedge* u) const { return pts[u->target()] - pts[u->vertex]; }

    // --- Support routines (2D projection to find initial supporting line)

    bool towards_support(int a, int b, int c) const {
        auto u = icross(a, b, c);
        return make_tuple(u.z, u.y, -u.x) > make_tuple(0, 0, 0);
    }

    auto turn_support_ccw(Wedge* u) const {
        Wedge *v = u->rotccw(), *first = u;
        u = v;
        while (u->rotccw() != first) {
            u = u->rotccw();
            v = towards_support(u->vertex, u->target(), v->target()) ? v : u;
        }
        return v;
    }

    auto turn_support_cw(Wedge* u) const {
        Wedge *v = u->rotcw(), *first = u;
        u = v;
        while (u->rotcw() != first) {
            u = u->rotcw();
            v = towards_support(u->vertex, v->target(), u->target()) ? v : u;
        }
        return v;
    }

    bool can_see_underneath_ccw(Wedge* A, int d) const {
        int a = A->vertex, b = A->target();
        return rank[a] > rank[b] && !towards_support(d, b, a);
    }

    bool can_see_underneath_cw(Wedge* D, int a) const {
        int d = D->vertex, c = D->target();
        return rank[d] < rank[c] && !towards_support(a, d, c);
    }

    // --- Main loop predicates

    bool advance_ccw(Wedge* A, Wedge* F, int d, const Pt3& up) const {
        int a = A->vertex, b = A->target();
        // Should we advance A to A->rnext() because A->rnext() is coplanar? Follow:
        // Don't advance from crossing, to old retreat, to backward or non-coplanar edge
        return A->mark != run && A->rnext() != F && dot(up, vec(A)) == 0 &&
               dot(up, icross(a, d, b)) > 0;
    }

    bool advance_cw(Wedge* D, Wedge* G, int a, const Pt3& up) const {
        int d = D->vertex, c = D->target();
        // Should we advance D to D->next because D->next is coplanar? Follow:
        // Don't advance from crossing, to old retreat, to backward or non-coplanar edge
        return D->mark != run && D->next != G && dot(up, vec(D)) == 0 &&
               dot(up, icross(a, d, c)) > 0;
    }

    bool retreat_ccw(Wedge* A, int d, const Pt3& up) const {
        int a = A->vertex, b = A->target();
        // Should we retreat A because coplanar D shadows it?
        return A->mark != run && dot(up, icross(a, d, b)) <= 0;
    }

    bool retreat_cw(Wedge* D, int a, const Pt3& up) const {
        int d = D->vertex, c = D->target();
        // Should we retreat D because coplanar A shadows it?
        return D->mark != run && dot(up, icross(a, d, c)) <= 0;
    }

    bool rotate_ccw(Wedge* A, Wedge* F, int d) const {
        int a = A->vertex, b = A->target(), f = A->rotccw()->target();
        // Should we rotate A to A->rotccw() because A->rotccw() is "above" A? Follow:
        // Don't rotate from crossing, from hanging, to F, or to lower plane
        return A->mark != run && b != f && A->rotccw() != F && orient(a, d, b, f) >= 0;
    }

    bool rotate_cw(Wedge* D, Wedge* G, int a) const {
        int d = D->vertex, c = D->target(), g = D->rotcw()->target();
        // Should we rotate D to D->rotcw() because D->rotcw() is "above" D? Follow:
        // Don't rotate from crossing, from hanging, to G, or to lower plane
        return D->mark != run && c != g && D->rotcw() != G && orient(a, d, c, g) >= 0;
    }

    auto recurse(int l, int r) const {
        if (l + 2 == r) {
            int a = index[l], b = index[l + 1];
            auto A = Wedge::loop(a, b);
            return make_pair(A, A->mate);
        } else if (l + 3 == r) {
            int a = index[l], b = index[l + 1], c = index[l + 2];
            if (collinear(pts[a], pts[b], pts[c])) {
                auto A = Wedge::loop(a, c);
                return make_pair(A, A->mate);
            } else if (towards_support(a, b, c)) {
                auto A = Wedge::triangle(a, b, c);
                return make_pair(A, A->next->mate);
            } else {
                auto A = Wedge::triangle(a, c, b);
                return make_pair(A, A->mate);
            }
        }

        // B[left hull]A   D[right hull]C
        int m = (l + r) / 2;
        auto [B, A] = recurse(l, m);
        auto [D, C] = recurse(m, r);
        ++run;

        // Walk along the 2D hull as seen by support to find the lower supporting line
        while (true) {
            if (can_see_underneath_ccw(A, D->vertex)) {
                A = turn_support_ccw(A->mate);
            } else if (can_see_underneath_cw(D, A->vertex)) {
                D = turn_support_cw(D->mate);
            } else {
                break;
            }
        }

        // Flip the edges so they point inwards into the hull, akin to delaunay
        A = turn_support_cw(A);
        D = turn_support_ccw(D);

        // Don't rotate A back to F, and don't rotate D back to G (wraparound edges)
        auto F = A, G = D;

        // Rotate A ccw for as long as A->rotccw() 'shadows' A along AD. Don't cut!
        while (rotate_ccw(A, F, D->vertex)) {
            A = A->rotccw();
        }
        // Rotate D cw for as long as D->rotcw() 'shadows' D along AD. Don't cut!
        while (rotate_cw(D, G, A->vertex)) {
            D = D->rotcw();
        }

        auto E = Wedge::connect(A->mate, D);
        B = B->vertex == E->vertex ? E : B;
        C = C->vertex == D->vertex ? E->mate : C;
        E->mark = E->mate->mark = run;

        while (true) {
            int a = A->vertex, b = A->target(), c = D->target(), d = D->vertex;
            int side = A->mark == run ? +1 : D->mark == run ? -1 : orient(a, d, b, c);

            if (side == -1) {
                auto up = icross(a, d, b);
                F = E;
                do {
                    A = A->rnext();
                } while (advance_ccw(A, F, d, up));
            } else if (side == +1) {
                auto up = icross(a, d, c);
                G = E->mate;
                do {
                    D = D->next;
                } while (advance_cw(D, G, a, up));
            } else {
                auto up = icross(a, d, b) + icross(a, d, c);
                F = E, G = E->mate;
                while (true) {
                    if (advance_ccw(A, F, D->vertex, up)) {
                        A = A->rnext();
                        while (retreat_cw(D->prev, A->vertex, up)) {
                            G = D, D = D->prev;
                        }
                    } else if (advance_cw(D, G, A->vertex, up)) {
                        D = D->next;
                        while (retreat_ccw(A->rprev(), D->vertex, up)) {
                            F = A, A = A->rprev();
                        }
                    } else {
                        break;
                    }
                }
            }

            F = A, G = D;

            // Cut and rotate A ccw for as long as A->rotccw() 'shadows' A along AD
            while (rotate_ccw(A, F, D->vertex)) {
                A = Wedge::cut_ccw(A);
            }
            // Cut and rotate D cw for as long as D->rotcw() 'shadows' D along AD
            while (rotate_cw(D, G, A->vertex)) {
                D = Wedge::cut_cw(D);
            }
            // If D reached crossing, clean up A
            while (A->vertex == D->target() && A != D->mate) {
                A = Wedge::cut_ccw(A);
            }
            // If A reached crossing, clean up D
            while (A->target() == D->vertex && A != D->mate) {
                D = Wedge::cut_cw(D);
            }
            if (A == D->mate) {
                break; // crossing
            }

            E = Wedge::connect(A->mate, D);
            E->mark = E->mate->mark = run;
        }

        return make_pair(B, C);
    }

    auto solve() const { return recurse(0, index.size()).second; }

    static Wedge* compute(const vector<Pt3>& pts) {
        assert(is_integral_v<Pt3::T> && "Only for integer points");
        dachull solver(pts);
        Wedge::use_temporary_pool();
        return Wedge::clone_all(solver.solve(), true);
    }
};
