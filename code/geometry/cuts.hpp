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
