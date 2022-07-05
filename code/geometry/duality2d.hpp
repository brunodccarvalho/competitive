#pragma once

#include "geometry/geometry2d.hpp"
#include "numeric/quot.hpp"

// Tools for exact computations with integer coordinates.
struct Duality {
    // The dual of point P(a,b) is y=ax-b. The dual of line y=cx-d is P(c,d).
    using Stab = quot<Pt2::T>;
    static inline const Stab INF = Stab(+1, 0), NINF = Stab(-1, 0);

    // Abscissa of intersection of dual lines f and g
    static auto isect(Pt2 f, Pt2 g) {
        return f.x != g.x ? Stab(f.y - g.y, f.x - g.x) : INF;
    }

    // Compare dual lines f(x)=ax-b & g(x)=cx-d at vertical x.
    static bool stab_compare(Pt2 f, Pt2 g, Stab x) {
        if (auto i = Stab::infsign(x)) {
            return i == +1 ? conj(f) < conj(g) : conj(f) > conj(g);
        } else if (f.x == g.x) {
            return f.y > g.y;
        } else {
            return Pt2::L(f.x - g.x) * x.n < Pt2::L(f.y - g.y) * x.d;
        }
    }

    // Do dual lines f and g intersect at x?
    static bool stab_equal(Pt2 f, Pt2 g, Stab x) {
        return x.d == 0 ? f == g : Pt2::L(f.x - g.x) * x.n == Pt2::L(f.y - g.y) * x.d;
    }

    // Compare dual lines f(x)=ax-b & g(x)=cx-d at vertical stab x±epsilon
    static bool stab_compare_epsilon(Pt2 f, Pt2 g, Stab x, bool after) {
        if (auto i = Stab::infsign(x)) {
            return i == +1 ? conj(f) < conj(g) : conj(f) > conj(g);
        } else if (f.x == g.x) {
            return f.y > g.y;
        } else if (auto dfg = Pt2::L(f.x - g.x) * x.n - Pt2::L(f.y - g.y) * x.d) {
            return dfg < 0;
        } else {
            return after ? f.x < g.x : f.x > g.x;
        }
    }
};
