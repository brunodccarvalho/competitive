#pragma once

#include "formatting.hpp"
#include "random.hpp"
#include "geometry/geometry3d.hpp"
#include "geometry/format3d.hpp"

auto bounding_box_size(const vector<Pt3>& pts) {
    int N = pts.size();
    if (N == 0) {
        return Pt3();
    }
    Pt3 a = pts[0], b = pts[0];
    for (int i = 1; i < N; i++) {
        for (int j = 0; j < 3; j++) {
            a[j] = min(a[j], pts[i][j]);
            b[j] = max(b[j], pts[i][j]);
        }
    }
    return b - a;
}

auto check_hull_exact(const vector<Pt3>& pts, const vector<vector<int>>& faces,
                      bool sameline = false, bool sameplane = false) {
    int F = faces.size(), N = pts.size();

    auto generic = [&](int dim, int face = -1, int point = -1) {
        string D = dim == -1 ? "?"s : dim == 0 ? "2/3"s : to_string(dim);
        println("-- {} points, {} faces, {}D hull", N, F, D);
        if (point != -1) {
            println(" Point {}: {}", point, pts[point]);
        } else if (N <= 20) {
            println(" Points:");
            for (int i = 0; i < N; i++) {
                println("  Point [{}]: {}", i, pts[i]);
            }
        }
        if (face != -1) {
            println(" Face {}: [{}]", face, faces[face]);
        } else if (F <= 20) {
            println(" Faces:");
            for (int f = 0; f < F; f++) {
                println("  Face [{}]: {}", f, faces[f]);
            }
        }
        return false;
    };
    auto get = [&](int face, int i) {
        int K = faces[face].size();
        return faces[face][i >= K ? i - K : i];
    };

    if (F == 0) {
        println("Error: hull is empty");
        return generic(-1);
    }

    vector<int> appearances(N);

    // Check that faces are not degenerate and points are within the range
    for (int f = 0; f < F; f++) {
        int S = faces[f].size();

        if (S == 0) {
            println("Error: face {} is empty", f);
            return generic(-1);
        }
        if (S == 1) {
            println("Error: face {}=[{}] has one point", f, faces[f][0]);
            return generic(-1, f, faces[f][0]);
        }
        for (int v : faces[f]) {
            assert(0 <= v && v < N);
            appearances[v]++;
        }
        for (int v : faces[f]) {
            if (appearances[v] > 1) {
                println("Error: point {} repeated x{} in face {}", v, appearances[v], f);
                return generic(-1, f, v);
            }
        }
        for (int v : faces[f]) {
            appearances[v]--;
        }
    }

    // Handle apparent 1D hull
    if (F == 1) {
        int S = faces[0].size(), f = 0;

        // 1D hull has exactly two vertices if !sameline
        if (!sameline && S != 2) {
            println("Error: hull face 0 should have exactly 2 vertices");
            return generic(1, 0);
        }

        int x = faces[0].front(), y = faces[0].back();

        // Check every point is indeed in the segment
        for (int u = 0; u < N; u++) {
            if (!onsegment(pts[x], pts[u], pts[y])) {
                println("Error (1D): hull={}-{}, but point {} is out of it", x, y, u);
                return generic(1, f, y);
            }
        }

        // Check the points are properly ordered
        for (int i = 0; i + 2 < S; i++) {
            int a = get(0, i), b = get(0, i + 1), c = get(0, i + 2);
            if (!onsegment(pts[a], pts[b], pts[c])) {
                println("Error (1D): {}-{}-{} not a segment", a, b, c);
                return generic(1, f, b);
            }
        }

        // OK 1D hull accepted
        return true;
    }

    // Check proper face sizes, triangulation, and non-collinear points
    for (int f = 0; f < F; f++) {
        int S = faces[f].size();
        if (S <= 2) {
            println("Error (2/3D): Face {}=[{}] is too small", f, faces[f]);
            return generic(0, -1, faces[f][0]);
        }
        if (sameline) {
            continue;
        }
        for (int i = 0; i < S; i++) {
            int a = get(f, i), b = get(f, i + 1), c = get(f, i + 2);

            if (collinear(pts[a], pts[b], pts[c])) {
                println("Error (2/3D): {}-{}-{} on face {} are collinear", a, b, c, f);
                return generic(0, f, b);
            }
        }
    }

    if (F == 3) {
        println("Error (3D): Hull can't have 3 faces");
        return generic(3);
    }

    unordered_map<pair<int, int>, tuple<int, int>> edges;
    vector<int> inface(N);

    for (int f = 0; f < F; f++) {
        int S = faces[f].size();
        int x = -1, y = -1, z = -1;
        Pt3 normal;

        // Find three non-collinear points
        for (int i = 1; i + 1 < S && normal == Pt3(); i++) {
            int a = get(f, 0), b = get(f, i), c = get(f, i + 1);
            normal = cross(pts[a], pts[b], pts[c]), x = a, y = b, z = c;
        }
        if (normal == Pt3()) {
            println("Error (2/3D): all points in face {} are collinear", f);
            return generic(0, f, faces[f][0]);
        }

        // Check CCw order around normal
        for (int i = 0; i < S; i++) {
            int a = get(f, i), b = get(f, i + 1), c = get(f, i + 2);
            auto order = dot(normal, cross(pts[a], pts[b], pts[c]));
            if (!sameline && order == 0) {
                println("Error (2/3D): {}-{}-{} collinear in face {}", a, b, c, f);
                return generic(0, f, b);
            } else if (order < 0) {
                println("Error (2/3D): {}-{}-{} concave in face {}", a, b, c, f);
                return generic(0, f, b);
            }
        }

        // Check coplanar orientation for every edge and other two points if S<=8
        for (int i = 0; i < S; i++) {
            int a = get(f, i);
            if (dot(pts[a] - pts[x], normal) != 0) {
                println("Error (2/3D): {} not on face {}", a, f);
                return generic(0, f, a);
            }
        }

        // For every point not on the face, check it is below the face
        for (int i = 0; i < S; i++) {
            inface[faces[f][i]] = true;
        }

        for (int u = 0; u < N; u++) {
            if (!inface[u]) {
                if (orientation(pts[x], pts[y], pts[z], pts[u]) == +1) {
                    println("Error (2/3D): Eye {} can see face {}", u, f);
                    println("Pts: {},{},{},{}", x, y, z, u);
                    println("Cross: {}",
                            dot(pts[u] - pts[x], cross(pts[x], pts[y], pts[z])));
                    return generic(0, f, u);
                }
            }
        }

        for (int i = 0; i < S; i++) {
            int a = get(f, i), b = get(f, i + 1), c = get(f, i + 2);
            if (edges.count({a, b})) {
                println("Error (2/3D): Repeated edge {}-{}", a, b);
                return generic(0, f, a);
            }
            edges[{a, b}] = {f, c};
        }

        for (int i = 0; i < S; i++) {
            inface[faces[f][i]] = false;
        }
    }

    // Check reverse edges exist and that faces are strictly convex around them
    for (auto [edge, face] : edges) {
        auto [u, v] = edge;
        auto [f, a] = face;

        if (!edges.count({v, u})) {
            println("Error: Reverse edge of {}->{} not found", u, v);
            return generic(0, f, u);
        }

        auto [g, b] = edges.at({v, u});
        auto order = orientation(pts[u], pts[v], pts[a], pts[b]);

        if (!sameplane && F > 2 && order == 0) {
            println("Error (3D): Faces {} and {} appear to be coplanar", f, g);
            println(" Edge: {}<-{}-{}->{}", b, u, v, a);
            return generic(3, f, u);
        } else if (order == +1) {
            println("Error (3D): Faces {} and {} appear to be concave", f, g);
            println(" Edge: {}<-{}-{}->{}", b, u, v, a);
            return generic(3, f, u);
        }
    }

    // OK 2D or 3D hull
    return true;
}

auto check_hull_inexact(const vector<Pt3>& pts, const vector<vector<int>>& faces,
                        bool sameline = false) {
    int F = faces.size(), N = pts.size();

    auto generic = [&](int dim, int face = -1, int point = -1) {
        string D = dim == -1 ? "?"s : dim == 0 ? "2/3"s : to_string(dim);
        println("-- {} points, {} faces, {}D hull", N, F, D);
        if (point != -1) {
            println(" Point {}: {}", point, pts[point]);
        } else if (N <= 20) {
            println(" Points:");
            for (int i = 0; i < N; i++) {
                println("  Point [{}]: {}", i, pts[i]);
            }
        }
        if (face != -1) {
            println(" Face {}: [{}]", face, faces[face]);
        } else if (F <= 20) {
            println(" Faces:");
            for (int f = 0; f < F; f++) {
                println("  Face [{}]: {}", f, faces[f]);
            }
        }
        return false;
    };
    auto get = [&](int face, int i) {
        int K = faces[face].size();
        return faces[face][i >= K ? i - K : i];
    };
    auto volume = [&](int a, int b, int c, int d) {
        auto p = (pts[a] + pts[b] + pts[c] + pts[d]) / 4;
        auto da = dot(pts[a] - p, cross(pts[c], pts[b], pts[d]));
        auto db = dot(pts[b] - p, cross(pts[a], pts[c], pts[d]));
        auto dc = dot(pts[c] - p, cross(pts[b], pts[a], pts[d]));
        auto dd = dot(pts[d] - p, cross(pts[a], pts[b], pts[c]));
        return abs(da + db + dc + dd) / 24.0;
    };
    auto surface_area = [&](int a, int b, int c, int d) {
        auto da = norm(cross(pts[a], pts[b], pts[c]));
        auto db = norm(cross(pts[b], pts[c], pts[d]));
        auto dc = norm(cross(pts[c], pts[d], pts[a]));
        auto dd = norm(cross(pts[d], pts[a], pts[b]));
        return (da + db + dc + dd) / 2.0;
    };
    auto area = [&](int a, int b, int c) {
        auto da = norm(cross(pts[a], pts[b], pts[c]));
        auto db = norm(cross(pts[b], pts[c], pts[a]));
        auto dc = norm(cross(pts[c], pts[a], pts[b]));
        return (da + db + dc) / 6.0;
    };
    auto big_volume = [&](int a, int b, int c, int d, bool log = false) {
        auto vol = volume(a, b, c, d);
        auto are = surface_area(a, b, c, d);
        auto ratio = vol * vol / (are * are * are);
        if (!log) {
            return ratio > 3e-5;
        } else if (ratio > 3e-5) {
            println(" Volume: {}", vol);
            println(" Surface area: {}", are);
            println(" Ratio: {}", ratio);
            return true;
        } else {
            return false;
        }
    };
    auto big_area = [&](int a, int b, int c, bool log = false) {
        auto are = area(a, b, c);
        auto sides = dist(pts[a], pts[b]) + dist(pts[b], pts[c]) + dist(pts[c], pts[a]);
        auto ratio = are / (sides * sides);
        if (!log) {
            return ratio > 3e-3;
        } else if (ratio > 3e-3) {
            println(" Area: {}", are);
            println(" Perimeter: {}", sides);
            println(" Ratio: {}", ratio);
            return true;
        } else {
            return false;
        }
    };

    if (F == 0) {
        println("Error: hull is empty");
        return generic(-1);
    }

    vector<int> appearances(N);

    // Check that faces are not degenerate and points are within the range
    for (int f = 0; f < F; f++) {
        int S = faces[f].size();

        if (S == 0) {
            println("Error: face {} is empty", f);
            return generic(-1);
        }
        if (S == 1) {
            println("Error: face {}=[{}] has one point", f, faces[f][0]);
            return generic(-1, f, faces[f][0]);
        }
        for (int v : faces[f]) {
            assert(0 <= v && v < N);
            appearances[v]++;
        }
        for (int v : faces[f]) {
            if (appearances[v] > 1) {
                println("Error: point {} repeated x{} in face {}", v, appearances[v], f);
                return generic(-1, f, v);
            }
        }
        for (int v : faces[f]) {
            appearances[v]--;
        }
    }

    // Handle apparent 1D hull
    if (F == 1) {
        int S = faces[0].size(), f = 0;

        // 1D hull has exactly two vertices if !sameline
        if (!sameline && S != 2) {
            println("Error: hull face 0 should have exactly 2 vertices");
            return generic(1, f, faces[f][0]);
        }

        int x = faces[0].front(), y = faces[0].back();

        // Check every point is indeed in the segment
        for (int u = 0; u < N; u++) {
            if (!onsegment(pts[x], pts[u], pts[y]) && big_area(x, u, y)) {
                println("Error (1D): hull={}-{}, but point {} is out of it", x, y, u);
                big_area(x, u, y, true);
                return generic(1, f, y);
            }
        }

        // Check the points are properly ordered
        for (int i = 0; i + 2 < S; i++) {
            int a = get(0, i), b = get(0, i + 1), c = get(0, i + 2);
            if (!onsegment(pts[a], pts[b], pts[c]) && big_area(a, b, c)) {
                println("Error (1D): {}-{}-{} not onsegment", a, b, c);
                big_area(a, b, c, true);
                return generic(1, f, b);
            }
        }

        // OK 1D hull accepted
        return true;
    }

    // Check proper face sizes, triangulation, and non-collinear points
    for (int f = 0; f < F; f++) {
        int S = faces[f].size();
        if (S <= 2) {
            println("Error (2/3D): Face {} is too small", f);
            return generic(0, f, faces[f][0]);
        }
        if (sameline) {
            continue;
        }
        for (int i = 0; i < S; i++) {
            int a = get(f, i), b = get(f, i + 1), c = get(f, i + 2);

            if (collinear(pts[a], pts[b], pts[c]) && big_area(a, b, c)) {
                println("Error (2/3D): {}-{}-{} on face {} are collinear", a, b, c, f);
                return generic(0, f, b);
            }
        }
    }

    if (F == 3) {
        println("Error (3D): Hull can't have 3 faces");
        return generic(3);
    }

    unordered_map<pair<int, int>, tuple<int, int>> edges;
    vector<int> inface(N);

    for (int f = 0; f < F; f++) {
        int S = faces[f].size();

        Pt3 normal;
        for (int i = 1; i + 1 < S; i++) {
            int a = get(f, 0), b = get(f, i), c = get(f, i + 1);
            normal += cross(pts[a], pts[b], pts[c]);
        }
        if (normal == Pt3()) {
            println("Error (2/3D): all points in face {} are collinear", f);
            return generic(0, f, faces[f][0]);
        }

        // Check CCw order around normal
        for (int i = 0; i < S && S > 3; i++) {
            int a = get(f, i), b = get(f, i + 1), c = get(f, i + 2);
            auto up = dot(normal, cross(pts[a], pts[b], pts[c]));
            if (up < 0 && big_area(a, b, c)) {
                println("Error (2/3D): {}-{}-{} in face {} are concave", a, b, c, f);
                big_area(a, b, c, true);
                return generic(0, f, b);
            }
        }

        // Check coplanar orientation for every edge and other two points if S<=8
        for (int i = 0; i < S && S <= 8; i++) {
            for (int j = 0; j < S; j++) {
                for (int k = 0; k < S; k++) {
                    int a = get(f, i), b = get(f, i + 1), c = get(f, j), d = get(f, k);
                    auto order = orientation(pts[a], pts[b], pts[c], pts[d]);

                    if (order != 0 && big_volume(a, b, c, d)) {
                        println("Error (2/3D): {}-{}-{}-{} bad orientation", a, b, c, d);
                        big_volume(a, b, c, d, true);
                        return generic(0, f, b);
                    }
                }
            }
        }

        // Check coplanar orientation for every edge pair if S>8
        for (int i = 0; i < S && S > 8; i++) {
            for (int j = 0; j < S; j++) {
                int a = get(f, i), b = get(f, i + 1), c = get(f, j), d = get(f, j + 1);
                auto order = orientation(pts[a], pts[b], pts[c], pts[d]);

                if (order != 0 && big_volume(a, b, c, d)) {
                    println("Error (2/3D): {}-{}-{}-{} bad orientation", a, b, c, d);
                    big_volume(a, b, c, d, true);
                    return generic(0, f, a);
                }
            }
        }

        int x = -1, y = -1, z = -1;
        Pt3::L xyz = 0;

        // Find largest triangle with any three points if S<=8
        for (int i = 0; i < S && S <= 8; i++) {
            for (int j = i + 1; j < S; j++) {
                for (int k = j + 1; k < S; k++) {
                    int a = get(f, i), b = get(f, j), c = get(f, k);
                    auto abc = norm2(cross(pts[a], pts[b], pts[c]));

                    if (abc > xyz) {
                        x = a, y = b, z = c, xyz = abc;
                    }
                }
            }
        }

        // Find a large triangle by randomly checking triples of points if S>8
        for (int runs = 0; runs < 150 && S > 8; runs++) {
            auto sample = int_sample<int>(3, 0, S);
            int a = get(f, sample[0]), b = get(f, sample[1]), c = get(f, sample[2]);
            auto abc = norm2(cross(pts[a], pts[b], pts[c]));

            if (abc > xyz) {
                x = a, y = b, z = c, xyz = abc;
            }
        }

        // For every point not on the face, check it is below the face
        for (int i = 0; i < S; i++) {
            inface[faces[f][i]] = true;
        }

        for (int u = 0; u < N; u++) {
            if (!inface[u]) {
                auto order = orientation(pts[x], pts[y], pts[z], pts[u]);

                if (order == +1 && big_volume(x, y, z, u)) {
                    println("Error (2/3D): Eye {} can see face {}", u, f);
                    big_volume(x, y, z, u, true);
                    return generic(0, f, u);
                }
            }
        }

        for (int i = 0; i < S; i++) {
            int a = get(f, i), b = get(f, i + 1), c = get(f, i + 2);
            if (edges.count({a, b})) {
                println("Error (2/3D): Repeated edge {}-{}", a, b);
                return generic(0, f, a);
            }
            edges[{a, b}] = {f, c};
        }

        for (int i = 0; i < S; i++) {
            inface[faces[f][i]] = false;
        }
    }

    // Check reverse edges exist
    for (auto [edge, face] : edges) {
        auto [u, v] = edge;
        auto [f, a] = face;

        if (!edges.count({v, u})) {
            println("Error: Reverse edge of {}->{} not found", u, v);
            return generic(0, f, u);
        }
    }

    // OK 2D or 3D hull
    return true;
}

// Check given hull (1d/2d/3d) with faces ccw pointing outwards
// - sameline: allow two consecutive edges to be collinear
// - sameplane: allow two adjacent faces to be coplanar
// Writes error to standard output. Quadratic complexity.
bool check_hull(const vector<Pt3>& pts, const vector<vector<int>>& faces,
                bool sameline = false, bool sameplane = false) {
    if constexpr (Pt3::FLOAT) {
        return check_hull_inexact(pts, faces, sameline);
    } else {
        return check_hull_exact(pts, faces, sameline, sameplane);
    }
}
