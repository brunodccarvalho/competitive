#pragma once

#include "geometry/geometry2d.hpp"
#include "numeric/quot.hpp"

// Tools for exact computations with integer coordinates.
// The implementation simulates the slope line moving from north to south in the plane
struct Duality {
    // The dual of point P(a,b) is y=ax-b. The dual of line y=cx-d is P(c,d).
    using Stab = quot<Pt2::L, true>;
    static inline const Stab INF = Stab(+1, 0), NINF = Stab(-1, 0);

    // Abscissa of intersection of dual lines f and g
    static auto slope(Pt2 f, Pt2 g) {
        return f.x != g.x ? Stab(f.y - g.y, f.x - g.x) : INF;
    }

    // Compare dual lines f(x)=ax-b & g(x)=cx-d at vertical x.
    static bool compare(Pt2 f, Pt2 g, Stab x) {
        if (auto i = infsign(x)) {
            return i == +1 ? conj(f) < conj(g) : conj(f) > conj(g);
        } else if (auto dt = Pt2::L(f.x - g.x) * x.n - Pt2::L(f.y - g.y) * x.d) {
            return dt < 0;
        } else {
            return f.x < g.x;
        }
    }

    // Do dual lines f and g intersect at x?
    static bool equal(Pt2 f, Pt2 g, Stab x) {
        return x.d == 0 ? f.x == g.x : Pt2::L(f.x - g.x) * x.n == Pt2::L(f.y - g.y) * x.d;
    }
};
