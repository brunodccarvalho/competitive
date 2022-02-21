#pragma once

#include "geometry/geometry2d.hpp"

/**
 * Perform a rotating calipers sweep with two orthogonal support lines (claws)
 * Visit point/point pairs for which the orthogonal calipers hang on, at a moment in time
 * where at least one of the calipers (and possibly both) perfectly aligns with an edge.
 * The calipers will rotate a total of 360º.
 * Complexity: O(N)
 * Signature:
 *    callback(u1,u2,v1,v2)
 *              u1,v1 are the supporting points for first and second calipers
 *              u2 is -1 or the vertex after u1 if the caliper stands on [u1u2]
 *              v2 is -1 or the vertex after v1 if the caliper stands on [v1v2]
 * Requires >=3 points and strictly convex ccw hull
 */
template <typename Visitor>
void orthogonal_calipers_sweep(const vector<Pt2>& hull, Visitor&& callback) {
    int N = hull.size();
    assert(N >= 3);

    auto perp90 = [&](int a, int b, int c, int d) {
        return dot(hull[b] - hull[a], hull[d] - hull[c]);
    };

    int u1 = 0, v1 = 0;
    while (perp90(N - 1, 0, v1, v1 + 1) >= 0) {
        v1++;
    }

    while (u1 != N) {
        int u2 = u1 + 1 == N ? 0 : u1 + 1;
        int v2 = v1 + 1 == N ? 0 : v1 + 1;
        auto rot = perp90(u1, u2, v1, v2);
        if (rot > 0) { // caliper hits v1+1 first
            callback(u1, -1, v1, v2);
            v1 = v2;
        } else if (rot < 0) { // caliper hits u1+1 first
            callback(u1, u2, v1, -1);
            u1++;
        } else { // orthogonal sides, caliper hits both u1+1 and v1+1
            callback(u1, u2, v1, v2);
            u1++, v1 = v2;
        }
    }
}

/**
 * Perform a rotating calipers sweep with two antipodal parallel support lines (bolts)
 * Visit point/point pairs for which the parallel calipers hang on, at a moment in time
 * where at least one of the calipers (and possibly both) perfectly aligns with an edge.
 * The calipers will rotate a total of 180º.
 * Complexity: O(N)
 * Signature:
 *    callback(u1,u2,v1,v2)
 *              u1,v1 are the supporting points for first and second calipers
 *              u2 is -1 or the vertex after u1 if the caliper stands on [u1u2]
 *              v2 is -1 or the vertex after v1 if the caliper stands on [v1v2]
 * Requires >=3 points and strictly convex ccw hull
 */
template <typename Visitor>
void parallel_calipers_sweep(const vector<Pt2>& hull, Visitor&& callback) {
    int N = hull.size();
    assert(N >= 3);

    auto perp180 = [&](int a, int b, int c, int d) {
        return cross(hull[b] - hull[a], hull[d] - hull[c]);
    };

    int u1 = 0, v1 = 1;
    while (perp180(N - 1, u1, v1, v1 + 1) >= 0) {
        v1++;
    }

    while (v1 != N) {
        int u2 = u1 + 1 == N ? 0 : u1 + 1;
        int v2 = v1 + 1 == N ? 0 : v1 + 1;
        auto rot = perp180(u1, u2, v1, v2);
        if (rot > 0) { // caliper hits v1+1 first
            callback(u1, -1, v1, v2);
            v1++;
        } else if (rot < 0) { // caliper hits u1+1 first
            callback(u1, u2, v1, -1);
            u1++;
        } else { // parallel sides, caliper hits both u1+1 and v1+1
            callback(u1, u2, v1, v2);
            u1++, v1++;
        }
    }
}

/**
 * Perform a rotating calipers sweep with four orthogonal support lines (bounding box)
 * Visit, in order, the N bounding boxes where the first caliper perfectly aligns with one
 * of the sides of the polygon.
 * The calipers will rotate a total of 360º.
 * Complexity: O(N)
 * Signature:
 *   functor(u1,u2,a,a',b,b',c,c')    u1u2 is the base edge
 *      a/b/c appear in ccw order after (possibly repeated)
 *      a' etc. is either -1 or (a+1)%N, the second indicating the edge a-a' overlaps
 *      the corresponding supporting line
 * Requires >=3 points and strictly convex ccw hull
 */
template <typename Visitor>
void box_calipers_sweep(const vector<Pt2>& hull, Visitor&& callback) {
    int N = hull.size();
    assert(N >= 3);

    auto perp90 = [&](int a, int b, int c, int d) {
        return dot(hull[b] - hull[a], hull[d] - hull[c]);
    };
    auto perp180 = [&](int a, int b, int c, int d) {
        return cross(hull[b] - hull[a], hull[d] - hull[c]);
    };
    auto perp270 = [&](int a, int b, int c, int d) {
        return dot(hull[a] - hull[b], hull[d] - hull[c]); // perp90(b,a,c,d)
    };
    auto fit = [&](int i) { return i >= N ? i - N : i; };

    // consider [i(i+1)] as base
    int u = 1, v = 1, w = 1;
    for (int i = 0; i < N; i++) {
        int l = fit(i + 1);

        // advance u while it makes <=90º
        auto urot = perp90(i, l, u, fit(u + 1));
        while (urot > 0) {
            v = fit(v + (u == v)); // keep v in front of u
            w = fit(w + (u == w)); // keep w in front of u
            u = fit(u + 1), urot = perp90(i, l, u, fit(u + 1));
        }
        int u2 = urot < 0 ? -1 : fit(u + 1);

        // advance v while it makes <=180º
        auto vrot = perp180(i, l, v, fit(v + 1));
        while (vrot > 0) {
            w = fit(w + (v == w)); // keep w in front of v
            v = fit(v + 1), vrot = perp180(i, l, v, fit(v + 1));
        }
        int v2 = vrot < 0 ? -1 : fit(v + 1);

        // advance w while it makes <=270º
        auto wrot = perp270(i, l, w, fit(w + 1));
        while (wrot > 0) {
            w = fit(w + 1), wrot = perp270(i, l, w, fit(w + 1));
        }
        int w2 = wrot < 0 ? -1 : fit(w + 1);

        // +-----v2?----v-------+
        // |                    |
        // w                    u2?
        // w2?                  u
        // |                    |
        // +------i------l------+
        callback(i, l, u, u2, v, v2, w, w2);
    }
}

/**
 * Perform a rotating calipers sweep with two parallel support lines (bolts) around two
 * convex polygons simultaneously. Everything identical to the singular case, except v1/v2
 * now lie in the second polygon.
 * If antipodal is true then the support lines start in "opposite" sides, otherwise they
 * start in the same side.
 */
template <bool antipodal, typename Visitor>
void parallel_calipers_sweep(const vector<Pt2>& uhull, const vector<Pt2>& vhull,
                             Visitor&& callback) {
    int N = uhull.size(), M = vhull.size();
    assert(N >= 3 && M >= 3);

    auto perp180 = [&](int a, int b, int c, int d) {
        if (antipodal) {
            return cross(uhull[b] - uhull[a], vhull[d] - vhull[c]);
        } else {
            return cross(uhull[b] - uhull[a], vhull[c] - vhull[d]);
        }
    };

    int u0 = min_element(begin(uhull), end(uhull)) - begin(uhull);
    int v0 = antipodal ? max_element(begin(vhull), end(vhull)) - begin(vhull)
                       : min_element(begin(vhull), end(vhull)) - begin(vhull);
    int u1 = u0, v1 = v0;

    do {
        int u2 = u1 + 1 == N ? 0 : u1 + 1;
        int v2 = v1 + 1 == M ? 0 : v1 + 1;
        auto rot = perp180(u1, u2, v1, v2);
        if (rot > 0) { // caliper hits v1+1 first
            callback(u1, -1, v1, v2);
            v1 = v2;
        } else if (rot < 0) { // caliper hits u1+1 first
            callback(u1, u2, v1, -1);
            u1 = u2;
        } else { // parallel sides, caliper hits both u1+1 and v1+1
            callback(u1, u2, v1, v2);
            u1 = u2, v1 = v2;
        }
    } while (u1 != u0 || v1 != v0);
}

auto all_antipodal_points(const vector<Pt2>& hull) {
    int N = hull.size();
    vector<array<int, 2>> antipodes;
    if (N == 2) {
        antipodes.push_back({0, 1});
    }
    if (N <= 2) {
        return antipodes;
    }

    auto area = [&](int a, int b, int c) { return cross(hull[a], hull[b], hull[c]); };

    int i = 0, j = 1;
    while (area(N - 1, i, j + 1) >= area(N - 1, i, j)) {
        j++;
    }

    while (j != N) {
        antipodes.push_back({i, j});
        // advance i or j?
        int k = j + 1 == N ? 0 : j + 1;
        int l = i + 1 == N ? 0 : i + 1;
        auto a = area(i, l, k);
        auto b = area(i, l, j);
        if (a > b) { // caliper hits j+1 first
            j++;
        } else if (a < b) { // caliper hits i+1 first
            i++;
        } else { // parallel sides
            antipodes.push_back({i, k});
            antipodes.push_back({l, j});
            i++, j++;
        }
    }

    return antipodes;
}

auto hull_minwidth(const vector<Pt2>& hull) {
    double ans = numeric_limits<double>::max();
    parallel_calipers_sweep(hull, [&](int u1, int u2, int v1, int v2) {
        if (u2 != -1) {
            ans = min(ans, linedist(hull[v1], hull[u1], hull[u2]));
        }
        if (v2 != -1) {
            ans = min(ans, linedist(hull[u1], hull[v1], hull[v2]));
        }
    });
    return ans;
}

auto hull_diameter(const vector<Pt2>& hull) {
    double ans = 0;
    for (auto [u, v] : all_antipodal_points(hull)) {
        ans = max(ans, dist(hull[u], hull[v]));
    }
    return ans;
}

auto min_area_oriented_bounding_box(const vector<Pt2>& hull) {
    double ans = numeric_limits<double>::max();
    box_calipers_sweep( //
        hull, [&](int u, int v, int a, int, int b, int, int c, int) {
            // compute a perpendicular to [uv]
            Pt2 x = hull[u] + perp_ccw(hull[v] - hull[u]);
            // project a and c on [ux]
            auto left = linedist(hull[a], hull[u], x);
            auto right = linedist(hull[c], hull[u], x);
            auto width = left + right;
            auto height = linedist(hull[b], hull[u], hull[v]);
            ans = min(ans, width * height);
        });
    return ans;
}

auto min_perimeter_oriented_bounding_box(const vector<Pt2>& hull) {
    double ans = numeric_limits<double>::max();
    box_calipers_sweep( //
        hull, [&](int u, int v, int a, int, int b, int, int c, int) {
            // compute a perpendicular to [uv], then a point on it above u
            Pt2 x = hull[u] + perp_ccw(hull[v] - hull[u]);
            // project a and c on [ux]
            auto left = linedist(hull[a], hull[u], x);
            auto right = linedist(hull[c], hull[u], x);
            auto width = left + right;
            auto height = linedist(hull[b], hull[u], hull[v]);
            ans = min(ans, 2 * (width + height));
        });
    return ans;
}
