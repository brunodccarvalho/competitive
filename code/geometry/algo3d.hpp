#pragma once

#include "geometry/geometry3d.hpp"

auto extract_points(const vector<vector<int>>& facets, const vector<Pt3>& pts) {
    int F = facets.size();
    vector<vector<Pt3>> ans(F);
    for (int f = 0; f < F; f++) {
        int N = facets[f].size();
        ans[f].resize(N);
        for (int i = 0; i < N; i++) {
            ans[f][i] = pts[facets[f][i]];
        }
    }
    return ans;
}

auto area3d(const vector<vector<int>>& facets, const vector<Pt3>& pts) {
    Pt3::L hull_area = 0;
    for (const auto& face : facets) {
        for (int i = 1, F = face.size(); i + 1 < F; i++) {
            const auto &a = pts[face[0]], b = pts[face[i]], c = pts[face[i + 1]];
            hull_area += norm(cross(a, b, c));
        }
    }
    return hull_area / 2.0;
}

auto volume3d(const vector<vector<int>>& facets, const vector<Pt3>& pts) {
    Pt3::L hull_volume = 0;
    for (const auto& face : facets) {
        for (int i = 1, F = face.size(); i + 1 < F; i++) {
            const auto &a = pts[face[0]], b = pts[face[i]], c = pts[face[i + 1]];
            hull_volume += dot(a, cross(b, c));
        }
    }
    return hull_volume / 6.0;
}

auto centroid3d(const vector<vector<int>>& facets, const vector<Pt3>& pts) {
    array<Pt3::L, 3> ans = {};
    auto sq = [](const Pt3::L& x) { return x * x; };
    for (const auto& face : facets) {
        for (int i = 1, F = face.size(); i + 1 < F; i++) {
            const auto &a = pts[face[0]], b = pts[face[i]], c = pts[face[i + 1]];
            auto n = cross(a, b, c);
            ans[0] += n.x * (sq(a.x + b.x) + sq(b.x + c.x) + sq(c.x + a.x));
            ans[1] += n.y * (sq(a.y + b.y) + sq(b.y + c.y) + sq(c.y + a.y));
            ans[2] += n.z * (sq(a.z + b.z) + sq(b.z + c.z) + sq(c.z + a.z));
        }
    }
    auto V = volume3d(facets, pts);
    ans[0] /= 48 * V, ans[1] /= 48 * V, ans[2] /= 48 * V;
    return ans;
}
