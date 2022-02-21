#pragma once

#include "geometry/geometry2d.hpp"

auto extract_points(const vector<int>& indices, const vector<Pt2>& pts) {
    int N = indices.size();
    vector<Pt2> ans(N);
    for (int i = 0; i < N; i++) {
        ans[i] = pts[indices[i]];
    }
    return ans;
}

auto two_oriented_area(const vector<Pt2>& poly) {
    Pt2::L ans = 0;
    for (int i = 0, N = poly.size(); i < N; i++) {
        auto u = poly[i ? i - 1 : N - 1], v = poly[i];
        ans += cross(u, v);
    }
    return ans;
}
auto two_area(const vector<Pt2>& poly) { return abs(two_oriented_area(poly)); }

auto perimeter(const vector<Pt2>& poly) {
    double ans = 0;
    for (int i = 0, N = poly.size(); i < N; i++) {
        auto u = poly[i ? i - 1 : N - 1], v = poly[i];
        ans += dist(u, v);
    }
    return ans;
}

auto centroid(const vector<Pt2>& poly) {
    array<double, 2> ans = {};
    for (int i = 0, N = poly.size(); i < N; i++) {
        auto u = poly[i ? i - 1 : N - 1], v = poly[i];
        ans[0] += double(u.x + v.x) * cross(u, v);
        ans[1] += double(u.y + v.y) * cross(u, v);
    }
    auto A = two_oriented_area(poly);
    ans[0] /= 6 * A, ans[1] /= 6 * A;
    return ans;
}
