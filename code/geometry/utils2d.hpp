#pragma once

#include "formatting.hpp"
#include "random.hpp"
#include "geometry/geometry2d.hpp"
#include "geometry/hull2d.hpp"
#include "geometry/format2d.hpp"
#include "geometry/cuts.hpp"

auto bounding_box_size(const vector<Pt2>& pts) {
    int N = pts.size();
    if (N == 0) {
        return Pt2();
    }
    Pt2 a = pts[0], b = pts[0];
    for (int i = 1; i < N; i++) {
        for (int j = 0; j < 2; j++) {
            a[j] = min(a[j], pts[i][j]);
            b[j] = max(b[j], pts[i][j]);
        }
    }
    return b - a;
}

// Check a triangulation for all points, with possibly some constrained segments
// The hull should be face[0] clockwise. Faces should be counterclockwise
bool check_constrained_triangulation(const vector<vector<int>>& faces,
                                     const vector<Pt2>& pts,
                                     const vector<array<int, 2>>& segments = {}) {
    int N = pts.size(), F = faces.size(), H = faces[0].size();

    set<array<int, 2>> edges;
    vector<int8_t> seen(N);

    // Check points lie inside the hull
    for (int i = 0; i < H; i++) {
        int u = faces[0][i], v = faces[0][i + 1 == H ? 0 : i + 1];
        for (int j = 0; j < N; j++) {
            if (orientation(pts[u], pts[v], pts[j]) == +1) {
                println("Point {} can see hull edge {}->{}", j, u, v);
                println("Hull: {}", faces[0]);
                return false;
            }
        }
        edges.insert({u, v});
        seen[u] = seen[v] = true;
    }

    // Check faces are CCW triangles, collect edges
    for (int f = 1; f < F; f++) {
        if (faces[f].size() != 3u) {
            println("Face {} is not a triangle", f);
            println("Face {}: [{}]", f, faces[f]);
            return false;
        }
        int u = faces[f][0], v = faces[f][1], w = faces[f][2];
        if (orientation(pts[u], pts[v], pts[w]) != +1) {
            println("Face {} is not a ccw triangle", f);
            println("Face {}: [{}]", f, faces[f]);
            return false;
        }
        edges.insert({u, v});
        edges.insert({v, w});
        edges.insert({w, u});
        seen[u] = seen[v] = seen[w] = true;
    }

    for (int u = 0; u < N; u++) {
        if (!seen[u]) {
            println("Point {} does not appear in the triangulation", u);
            return false;
        }
    }
    for (auto [u, v] : segments) {
        if (!edges.count({u, v})) {
            println("Constraint {}-{} does not appear in the triangulation", u, v);
            return false;
        }
    }
    for (auto [u, v] : edges) {
        if (u == v) {
            println("Self edge {}->{} found", u, v);
            return false;
        }
        if (!edges.count({v, u})) {
            println("Reverse edge of {}->{} not found", u, v);
            return false;
        }
    }

    vector<map<int, int>> incident(N);
    for (auto [u, v] : edges) {
        if (++incident[u][v] == 2) {
            println("Multiple edges {}->{} found", u, v);
            return false;
        }
    }

    for (auto [a, b] : edges) {
        for (int u = 0; u < N && a < b; u++) {
            if (u != a && u != b && onsegment(pts[a], pts[u], pts[b])) {
                println("Point {} lies on segment {}->{}", u, a, b);
                return false;
            }
        }
    }

    // Now check segments don't intersect. This is O(N^2)
    for (auto [u, v] : edges) {
        for (auto [a, b] : edges) {
            if (u < v && a < b && (u != a || v != b)) {
                if (segments_intersect(pts[a], pts[b], pts[u], pts[v])) {
                    println("Bad pair: ({},{}), ({},{})", u, v, a, b);
                    return false;
                }
            }
        }
    }

    return true;
}

// naive O(NH) verifier
bool check_hull_exact(const vector<Pt2>& pts, const vector<Pt2>& hull,
                      bool strict = false) {
    int N = pts.size(), H = hull.size();
    assert(H > 0 || N == 0);

    // all points are equal
    if (H == 1) {
        for (int i = 0; i < N; i++) {
            assert(pts[i] == hull[0]);
        }
        if (strict) { // the points in the hull must exist
            assert(N > 0);
        }
    }
    // all points lie on the segment hull[0]..hull[1]
    else if (H == 2) {
        for (int i = 0; i < N; i++) {
            assert(onsegment(hull[0], pts[i], hull[1]));
        }
        if (strict) { // the points must exist
            assert(hull[0] != hull[1]);
            assert(find(begin(pts), end(pts), hull[0]) != end(pts));
            assert(find(begin(pts), end(pts), hull[1]) != end(pts));
        }
    }
    // verify we make strict turns and points lie inside the convex hull
    else if (H >= 3) {
        for (int a = 0; a < H; a++) {
            int b = (a + 1) % H, c = (a + 2) % H;
            assert(orientation(hull[a], hull[b], hull[c]) > 0);
        }
        for (int i = 0; i < N; i++) {
            for (int a = 0; a < H; a++) {
                int b = (a + 1) % H;
                assert(orientation(hull[a], hull[b], pts[i]) >= 0);
            }
        }
    }

    return true;
}

void check_separating_line(const vector<Pt2>& reds, const vector<Pt2>& blues, Ray line,
                           int red_side) {
    int n = reds.size(), m = blues.size();
    for (int i = 0; i < n; i++) {
        int side = fiveway_orientation(reds[i], line, red_side);
        assert(side == +1 || side == +2);
    }
    for (int i = 0; i < m; i++) {
        int side = fiveway_orientation(blues[i], line, red_side);
        assert(side == -1 || side == -2);
    }
}

bool check_sandwich_line(const vector<Pt2>& reds, const vector<Pt2>& blues, Ray line) {
    int n = reds.size(), m = blues.size();
    array<int, 3> as = {}, bs = {};
    for (int i = 0; i < n; i++) {
        auto side = cross(line.d, reds[i] - line.p);
        as[0] += side > 0;
        as[1] += side == 0;
        as[2] += side < 0;
    }
    for (int i = 0; i < m; i++) {
        auto side = cross(line.d, blues[i] - line.p);
        bs[0] += side > 0;
        bs[1] += side == 0;
        bs[2] += side < 0;
    }

    int a = n / 2, b = m / 2;
    bool ok = true;
    ok &= as[0] <= a && bs[0] <= b; // proper sandwich on the left
    ok &= as[2] <= a && bs[2] <= b; // proper sandwich on the left
    ok &= as[1] >= 1 && bs[1] >= 1; // at least one of each color on the line
    if (!ok) {
        println("Bad sandwich line");
        println("reds:  {},{:3}", as, n);
        println("blues: {},{:3}", bs, m);
    }
    return ok;
}

bool check_approximate_sandwich_line(const vector<Pt2>& reds, const vector<Pt2>& blues,
                                     Ray line) {
    int n = reds.size(), m = blues.size();
    assert(n == m);
    array<int, 3> as = {}, bs = {};
    for (int i = 0; i < n; i++) {
        auto side = cross(line.d, reds[i] - line.p);
        as[0] += side > 0;
        as[1] += side == 0;
        as[2] += side < 0;
    }
    for (int i = 0; i < m; i++) {
        auto side = cross(line.d, blues[i] - line.p);
        bs[0] += side > 0;
        bs[1] += side == 0;
        bs[2] += side < 0;
    }

    bool ok = true;
    ok &= as[0] == bs[0];                   // same number of points on left
    ok &= as[2] == bs[2];                   // same number of points on right
    ok &= as[0] >= n / 4 && bs[0] >= n / 4; // decent number on each side
    ok &= as[1] >= 1 && bs[1] >= 1;         // line passes through 2 points at least
    if (!ok) {
        println("Bad approximate sandwich line");
        println("reds:  {},{:3}", as, n);
        println("blues: {},{:3}", bs, m);
    }
    return ok;
}
