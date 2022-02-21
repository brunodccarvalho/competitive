#pragma once

#include "geometry/geometry3d.hpp"

auto extract_points(const vector<vector<int>>& facets, const vector<P>& pts) {
    int F = facets.size();
    vector<vector<P>> ans(F);
    for (int f = 0; f < F; f++) {
        int N = facets[f].size();
        ans[f].resize(N);
        for (int i = 0; i < N; i++) {
            ans[f][i] = pts[facets[f][i]];
        }
    }
    return ans;
}

auto area3d(const vector<vector<int>>& facets, const vector<P>& pts) {
    P::D hull_area = 0;
    for (const auto& face : facets) {
        for (int i = 1, F = face.size(); i + 1 < F; i++) {
            const auto &a = pts[face[0]], b = pts[face[i]], c = pts[face[i + 1]];
            hull_area += norm(cross(a, b, c));
        }
    }
    return hull_area / 2.0;
}

auto volume3d(const vector<vector<int>>& facets, const vector<P>& pts) {
    P::D hull_volume = 0;
    for (const auto& face : facets) {
        for (int i = 1, F = face.size(); i + 1 < F; i++) {
            const auto &a = pts[face[0]], b = pts[face[i]], c = pts[face[i + 1]];
            hull_volume += dot(a, cross(b, c));
        }
    }
    return hull_volume / 6.0;
}

auto centroid3d(const vector<vector<int>>& facets, const vector<P>& pts) {
    P::DP ans = {};
    for (const auto& face : facets) {
        for (int i = 1, F = face.size(); i + 1 < F; i++) {
            const auto &a = pts[face[0]], b = pts[face[i]], c = pts[face[i + 1]];
            auto n = cross(a, b, c);
            ans[0] += n.x * (pow(a.x + b.x, 2) + pow(b.x + c.x, 2) + pow(c.x + a.x, 2));
            ans[1] += n.y * (pow(a.y + b.y, 2) + pow(b.y + c.y, 2) + pow(c.y + a.y, 2));
            ans[2] += n.z * (pow(a.z + b.z, 2) + pow(b.z + c.z, 2) + pow(c.z + a.z, 2));
        }
    }
    auto V = volume3d(facets, pts);
    ans[0] /= 48 * V, ans[1] /= 48 * V, ans[2] /= 48 * V;
    return ans;
}
