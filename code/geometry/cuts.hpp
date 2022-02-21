#pragma once

#include "geometry/geometry2d.hpp"
#include "geometry/duality2d.hpp"

// Given n red points and m blue points, find a separating line or determine none exists.
// Returns [found,red,blue]. O(n+m) with high probability. 30ms for ~500K points.
// Handles collinear points, but rejects if any pair of red and blue points coincide.
// If the pointsets are separable then there exist two tangent lines between their hulls.
// Set red_side to +1 to put reds on the left; set it to -1 to put reds on the right.
auto separating_line(const vector<Pt2>& reds, const vector<Pt2>& blues,
                     int red_side = +1) {
    int n = reds.size(), m = blues.size();
    assert(n > 0 && m > 0 && (red_side == +1 || red_side == -1));

    vector<int> index(n + m);
    iota(begin(index), end(index), 0);

    static mt19937 rng(random_device{}());
    using intd = uniform_int_distribution<int>;

    intd any_red(0, n - 1), any_blue(n, n + m - 1);
    swap(index[0], index[any_red(rng)]);
    swap(index[1], index[any_blue(rng)]);
    shuffle(begin(index) + 2, end(index), rng);

    // Start off with a line from first red point to first blue point, chosen randomly
    // If more red points lie on the line, they must lie 'before' red.
    // If more blue points lie on the line, they must lie 'after' blue.
    int red = index[0], blue = index[1] - n;
    Ray line = Ray::through(reds[red], blues[blue]);

    if (reds[red] == blues[blue]) {
        return make_tuple(false, red, blue); // D:
    }

    for (int i = 2; i < n + m; i++) {
        auto u = index[i] < n ? reds[index[i]] : blues[index[i] - n];
        int side = fiveway_orientation(u, line, red_side);

        if (index[i] < n) {   // red point
            if (side >= +1) { // point on the correct side
                continue;
            } else if (side == 0) { // tighten the gap
                line = Ray::through(u, line.q()), red = index[i];
                continue;
            } else if (side == -1) { // red-blue-red, nope!
                return make_tuple(false, red, blue);
            }

            auto w = line.p;
            line = Ray::through(u, line.q()), red = index[i];
            bool check = false;

            // If any blue point v lies inside [uwq] pivot on u and rotate from q to v
            for (int j = 0; j < i; j++) {
                if (index[j] >= n) {
                    auto v = blues[index[j] - n];
                    int vside = fiveway_orientation(v, line, red_side);
                    if (vside <= -1) {
                        continue;
                    } else if (vside == 0) { // tighten the gap
                        line = Ray::through(u, v), blue = index[j] - n;
                    } else if (vside == +2 && orientation(v, w, u) == red_side) {
                        line = Ray::through(u, v), blue = index[j] - n, check = true;
                    } else {
                        return make_tuple(false, red, blue);
                    }
                }
            }
            // Verify all red points lie strictly to the side of the line
            for (int j = 0; check && j < i; j++) {
                if (index[j] < n) {
                    auto v = reds[index[j]];
                    if (fiveway_orientation(v, line, red_side) != 2) {
                        return make_tuple(false, red, blue);
                    }
                }
            }
        } else {              // blue point
            if (side <= -1) { // point on the correct side
                continue;
            } else if (side == 0) { // tighten the gap
                line = Ray::through(line.p, u), blue = index[i] - n;
                continue;
            } else if (side == +1) { // blue-red-blue, nope!
                return make_tuple(false, red, blue);
            }

            auto w = line.q();
            line = Ray::through(line.p, u), blue = index[i] - n;
            bool check = false;

            // If any red point v lies inside [uwp] pivot on u and rotate from p to v
            for (int j = 0; j < i; j++) {
                if (index[j] < n) {
                    auto v = reds[index[j]];
                    int vside = fiveway_orientation(v, line, red_side);
                    if (vside >= +1) {
                        continue;
                    } else if (vside == 0) { // tighten the gap
                        line = Ray::through(v, u), red = index[j];
                    } else if (vside == -2 && orientation(v, w, u) == red_side) {
                        line = Ray::through(v, u), red = index[j], check = true;
                    } else {
                        return make_tuple(false, red, blue);
                    }
                }
            }
            // Verify all blue points lie strictly to the side of the line
            for (int j = 0; check && j < i; j++) {
                if (index[j] >= n) {
                    auto v = blues[index[j] - n];
                    if (fiveway_orientation(v, line, red_side) != -2) {
                        return make_tuple(false, red, blue);
                    }
                }
            }
        }
    }

    return make_tuple(true, red, blue); // :D
}

// Compute a line that simultaneously bisects red and blue points.
// Returns [red index,blue index]. O(N log N), 600ms for ~500K points.
// Handles collinear points but not coincident points. If #reds or #blues are even then
// the balance of reds/blues on each side is effectively uniformly random.
auto sandwich_line(const vector<Pt2>& reds, const vector<Pt2>& blues) {
    int n = reds.size(), m = blues.size();
    assert(n > 0 && m > 0);

    // We will pick a line with 'a' red points below it and 'b' blue points below it.
    int a = (n - 1) / 2, b = (m - 1) / 2;
    vector<int> A(n), B(m);
    iota(begin(A), end(A), 0);
    iota(begin(B), end(B), 0);

    static mt19937 rng(random_device{}());
    using intd = uniform_int_distribution<int>;

    // Discard points so that n and m are both odd for the odd intersection property
    if (n % 2 == 0) {
        n--, swap(A[intd(0, n)(rng)], A[n]), A.pop_back();
    }
    if (m % 2 == 0) {
        m--, swap(B[intd(0, m)(rng)], B[m]), B.pop_back();
    }

    // Shuffle to help with the intersections later and expected performance
    shuffle(begin(A), end(A), rng), shuffle(begin(B), end(B), rng);
    nth_element(begin(A), begin(A) + a, end(A), [&](int u, int v) {
        return Duality::stab_compare(reds[u], reds[v], Duality::INF);
    });
    nth_element(begin(B), begin(B) + b, end(B), [&](int u, int v) {
        return Duality::stab_compare(blues[u], blues[v], Duality::INF);
    });

    // There's a vertical sandwich cut, at this x, take it.
    if (reds[A[a]].x == blues[B[b]].x) {
        return make_pair(A[a], B[b]);
    }

    // We maintain an open vertical slab (L,R) on which lies an x for which the red and
    // blue median lines intersect. To split the slab, quickselect the dual lines along a
    // stab at x. If the two median lines intersect in this stab we're done. Otherwise we
    // trim the slab to (L,x) or (x,R), but then we must find another intersection point
    // inside the new slab. Suppose there are no red/red and no blue/blue intersections
    // inside the slab (L,R), then the only intersections are red/blue crossings. But this
    // implies the red and blue median lines inside the slab are completely straight, so
    // we would find their isect at L or R. So we keep track of some red/red and blue/blue
    // intersections during quickselect swaps in a pool of stabs, and choose which to keep
    // with reservoir sampling. We can still look for more stabs after with heuristics.
    static constexpr int pool = 9, target_stabs = 3;
    static Duality::Stab stabs[pool] = {};

    // The current slab and next vertical stab
    auto L = Duality::NINF, R = Duality::INF;
    auto x = Duality::isect(reds[A[a]], blues[B[b]]);
    bool inf_compare = Duality::stab_compare(reds[A[a]], blues[B[b]], Duality::INF);
    int bag = 0, reservoir = 0;

    auto insert_isect = [&](auto y) {
        if (L < y && y < R) {
            if (bag < pool) {
                stabs[bag++] = y, reservoir = bag;
            } else if (int i = intd(0, reservoir++)(rng); i < pool) {
                stabs[i] = y;
            }
        }
    };
    auto red_compare = [&](int u, int v) {
        insert_isect(Duality::isect(reds[u], reds[v]));
        return Duality::stab_compare(reds[u], reds[v], x);
    };
    auto blue_compare = [&](int u, int v) {
        insert_isect(Duality::isect(blues[u], blues[v]));
        return Duality::stab_compare(blues[u], blues[v], x);
    };

    while (true) {
        nth_element(begin(A), begin(A) + a, end(A), red_compare);
        nth_element(begin(B), begin(B) + b, end(B), blue_compare);

        // We found a median stab, we're done
        if (Duality::stab_equal(reds[A[a]], blues[B[b]], x)) {
            return make_pair(A[a], B[b]);
        }

        // Clean up the pool, keep only those that lie strictly inside the new slab
        if (Duality::stab_compare(reds[A[a]], blues[B[b]], x) == inf_compare) {
            auto mid = remove_if(stabs, stabs + bag, [&](auto y) { return y >= x; });
            reservoir = bag = mid - stabs, R = x;
        } else {
            auto mid = remove_if(stabs, stabs + bag, [&](auto y) { return y <= x; });
            reservoir = bag = mid - stabs, L = x;
        }

        // Look for more intersections in likely places until we've got plenty of stabs
        for (int i = 0; i < n && bag < target_stabs; i++) {
            insert_isect(Duality::isect(reds[A[i]], blues[B[b]]));
        } // all red dual lines with blue median line at x
        for (int i = 0; i < m && bag < target_stabs; i++) {
            insert_isect(Duality::isect(reds[A[a]], blues[B[i]]));
        } // all blue dual lines with red median line at x

        if (bag > 0) {
            nth_element(stabs, stabs + (bag / 2), stabs + bag);
            x = stabs[bag / 2], stabs[bag / 2] = stabs[bag - 1], bag--;
            continue;
        }

        // Since we have no interior stab, which sucks, just restart and try again
        shuffle(begin(A), end(A), rng), shuffle(begin(B), end(B), rng);
        nth_element(begin(A), begin(A) + a, end(A), [&](int u, int v) {
            return Duality::stab_compare(reds[u], reds[v], Duality::INF);
        });
        nth_element(begin(B), begin(B) + b, end(B), [&](int u, int v) {
            return Duality::stab_compare(blues[u], blues[v], Duality::INF);
        });

        L = Duality::NINF, R = Duality::INF;
        x = Duality::isect(reds[A[a]], blues[B[b]]);
    }
}

// Compute a line that splits n red and n blue points into (k,k) and (n-k-1,n-k-1)
// for some k>=n/4 (balanced split on average). O(n log n). ~200ms for 500K points
// No three points should be collinear, or bogus results may be returned.
auto approximate_sandwich_line(const vector<Pt2>& reds, const vector<Pt2>& blues) {
    int n = reds.size();
    assert(n > 0 && n == int(blues.size()));

    static mt19937 rng(random_device{}());
    using intd = uniform_int_distribution<int>;

    vector<int> A(n), B(n);
    iota(begin(A), end(A), 0);
    iota(begin(B), end(B), 0);

    while (true) {
        int red = intd(0, n - 1)(rng);

        sort(begin(A), end(A), [&](int u, int v) {
            return biangle_sort(reds[u] - reds[red], reds[v] - reds[red]);
        });
        sort(begin(B), end(B), [&](int u, int v) {
            return biangle_sort(blues[u] - reds[red], blues[v] - reds[red]);
        });

        int as = 0, bs = 0, blue = -1, best = -1;

        for (int i = 1; i < n; i++) {
            int above = quadrant(reds[A[i]] - reds[red]) < 3;
            as += above;
        }
        for (int j = 0; j < n; j++) {
            int above = quadrant(blues[B[j]] - reds[red]) < 3;
            bs += above;
        }

        auto advance_red = [&](int u) {
            int above = quadrant(reds[u] - reds[red]) < 3;
            as += above ? -1 : +1;
        };
        auto advance_blue = [&](int v) {
            int above = quadrant(blues[v] - reds[red]) < 3;
            bs += above ? -1 : 0;
            if (as == bs && min(as, n - 1 - as) > best) {
                blue = v, best = min(as, n - 1 - as);
            }
            bs += above ? 0 : +1;
        };

        // Advance to the next red or blue, depending on which we hit first
        int i = 1, j = 0;
        while (i < n && j < n) {
            if (biangle_sort(reds[A[i]] - reds[red], blues[B[j]] - reds[red])) {
                advance_red(A[i++]);
            } else {
                advance_blue(B[j++]);
            }
        }
        while (j < n) {
            advance_blue(B[j++]);
        }

        if (best >= n / 4) {
            return make_pair(red, blue);
        }
    }
}
