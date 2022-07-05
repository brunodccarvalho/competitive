#pragma once

#include "geometry/geometry2d.hpp"

/**
 * visit every pair of points, with all points sorted outline.
 * radial sweep ccw around the origin, starting at -Oy and ending at +Oy
 * there must be no coincident points. Collinear points are ok.
 * Signature:
 *    processor(const order&, int i, int u, int v)
 *      where we visit the pair (pts[u],pts[v]) with order[i] == u and all
 *      other points are sorted along the direction RH(pts[u],pts[v])
 */
template <typename Fn>
void all_point_pairs_radial_sweep(const vector<Pt2>& pts, Fn&& processor) {
    int N = pts.size();
    assert(N <= 10'000); // sanity check - we need O(N^2) memory

    vector<int> order(N);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order), [&](int u, int v) { return pts[u] < pts[v]; });

    vector<int> rank(N);
    for (int i = 0; i < N; i++) {
        rank[order[i]] = i;
    }

    int S = N * (N - 1) / 2;
    vector<array<int, 2>> slopes(S);
    for (int k = 0, i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++, k++) {
            slopes[k] = {order[i], order[j]};
        }
    }
    sort(begin(slopes), end(slopes), [&](const auto& u, const auto& v) {
        const auto &a = pts[u[0]], b = pts[u[1]], c = pts[v[0]], d = pts[v[1]];
        if (auto cuv = cross(b - a, d - c)) {
            return cuv > 0; // different slopes
        } else if (a != c) {
            return make_pair(a.x, a.y) < make_pair(c.x, c.y);
        } else {
            return make_pair(b.x, b.y) < make_pair(d.x, d.y);
        }
    });

    for (int i = 0; i < S; i++) {
        auto [u, v] = slopes[i];
        int a = rank[u], b = rank[v];
        if (a > b) {
            swap(a, b);
            swap(u, v);
        }
        assert(a + 1 == b);
        processor(order, a, u, v);
        swap(order[a], order[b]);
        swap(rank[u], rank[v]);
    }
}

auto smallest_triangle_area(const vector<Pt2>& pts) {
    Pt2::L ans = numeric_limits<Pt2::L>::max();
    all_point_pairs_radial_sweep(pts, [&](const auto& order, int i, int u, int v) {
        int N = pts.size();
        assert(order[i] == u);
        if (i > 0) {
            ans = min(ans, abs(cross(pts[order[i - 1]], pts[u], pts[v])));
        }
        if (i + 1 < N) {
            ans = min(ans, abs(cross(pts[order[i + 1]], pts[u], pts[v])));
        }
    });
    return ans;
}
