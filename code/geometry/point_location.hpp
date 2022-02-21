#pragma once

#include "geometry/geometry2d.hpp"
#include "geometry/wedge.hpp"
#include "geometry/shaft_scanner.hpp"
#include "geometry/duality2d.hpp"

// Given non-intersecting segments, for each input point determine the segment below and
// above this point closest to it vertically, or -1 if no such segment exists. O(N log N)
// The segments do not need to be connected or oriented. Coincident points allowed.
// If the query lies on an input point then that input point is ignored for this query.
// If the query lies on an input segment then you choose which vector the segment goes:
static constexpr int HIT_IGNORE = 0, HIT_ABOVE = 1, HIT_BELOW = 2;

auto offline_point_location(const vector<Pt2>& pts, const vector<array<int, 2>>& segments,
                            int hit_location = HIT_IGNORE, Pt2 forward = Pt2(1, 0)) {
    int N = pts.size(), S = segments.size();
    live_sweep_scanner scanner(forward);
    auto sorter = line_sorter(forward);

    vector<int> index(N), rank(N);
    iota(begin(index), end(index), 0);
    sort(begin(index), end(index), [&](int i, int j) { return sorter(pts[i], pts[j]); });

    for (int i = 0; i < N; i++) {
        rank[index[i]] = i;
    }

    vector<Ray> rays(S);
    vector<int> head(N, -1), next_head(S, -1);
    vector<int> tail(N, -1), next_tail(S, -1);
    for (int s = 0; s < S; s++) {
        auto [u, v] = segments[s];
        if (rank[u] < rank[v]) {
            rays[s] = Ray::through(pts[u], pts[v]);
            next_head[s] = head[u], head[u] = s;
            next_tail[s] = tail[v], tail[v] = s;
        } else {
            rays[s] = Ray::through(pts[v], pts[u]);
            next_head[s] = head[v], head[v] = s;
            next_tail[s] = tail[u], tail[u] = s;
        }
    }

    map<Ray, int, live_sweep_scanner> shafts(scanner);
    vector<int> below(N), above(N);

    auto answer_query = [&](int q) {
        if (hit_location == HIT_IGNORE) {
            auto above_shaft = shafts.lower_bound(pts[q]);
            auto below_shaft = shafts.upper_bound(pts[q]);
            above[q] = above_shaft == shafts.begin() ? -1 : prev(above_shaft)->second;
            below[q] = below_shaft == shafts.end() ? -1 : below_shaft->second;
        } else if (hit_location == HIT_BELOW) {
            auto shaft = shafts.lower_bound(pts[q]);
            above[q] = shaft == shafts.begin() ? -1 : prev(shaft)->second;
            below[q] = shaft == shafts.end() ? -1 : shaft->second;
        } else if (hit_location == HIT_ABOVE) {
            auto shaft = shafts.upper_bound(pts[q]);
            above[q] = shaft == shafts.begin() ? -1 : prev(shaft)->second;
            below[q] = shaft == shafts.end() ? -1 : shaft->second;
        } else {
            assert(false && "Invalid hit location");
        }
    };

    for (int l = 0, r = 1; r <= N; l = r++) {
        int u = index[l];
        while (r < N && pts[u] == pts[index[r]]) {
            r++;
        }
        for (int i = l; i < r; i++) {
            for (int v = index[i], s = tail[v]; s != -1; s = next_tail[s]) {
                shafts.erase(rays[s]);
            }
        }
        for (int i = l; i < r; i++) {
            answer_query(index[i]);
        }
        for (int i = l; i < r; i++) {
            for (int v = index[i], s = head[v]; s != -1; s = next_head[s]) {
                shafts.emplace(rays[s], s);
            }
        }
    }

    return make_pair(move(below), move(above));
}

// Given a connected PSLD, for each query point determine the PSLD edge below this point
// going 'rightwards' and closest to this point, or -1 if no such edge exists. O(M log M)
// The PSLD does not need to be convex or triangulated.
// Can be easily adapted for other sweep directions (just change the sort function)
auto offline_point_location(const vector<Pt2>& queries, const vector<Wedge*>& pslg,
                            const vector<Pt2>& pts, int hit_location = HIT_IGNORE,
                            Pt2 forward = Pt2(1, 0)) {
    int N = pts.size(), S = pslg.size();
    live_sweep_scanner scanner(forward);
    auto sorter = line_sorter(forward);

    vector<int> index(N), rank(N);
    iota(begin(index), end(index), 0);
    sort(begin(index), end(index), [&](int i, int j) { return sorter(pts[i], pts[j]); });

    for (int i = 0; i < N; i++) {
        rank[index[i]] = i;
    }

    vector<Ray> rays(S);
    vector<int> head(N, -1), next_head(S, -1);
    vector<int> tail(N, -1), next_tail(S, -1);
    for (int s = 0; s < S; s++) {
        int u = pslg[s]->vertex, v = pslg[s]->target();
        if (rank[u] < rank[v]) {
            rays[s] = Ray::through(pts[u], pts[v]);
            next_head[s] = head[u], head[u] = s;
            next_tail[s] = tail[v], tail[v] = s;
        }
    }

    map<Ray, int, live_sweep_scanner> shafts(scanner);
    vector<int> below(N), above(N);

    auto answer_query = [&](int q) {
        if (hit_location == HIT_IGNORE) {
            auto above_shaft = shafts.lower_bound(pts[q]);
            auto below_shaft = shafts.upper_bound(pts[q]);
            above[q] = above_shaft == shafts.begin() ? -1 : prev(above_shaft)->second;
            below[q] = below_shaft == shafts.end() ? -1 : below_shaft->second;
        } else if (hit_location == HIT_BELOW) {
            auto shaft = shafts.lower_bound(pts[q]);
            above[q] = shaft == shafts.begin() ? -1 : prev(shaft)->second;
            below[q] = shaft == shafts.end() ? -1 : shaft->second;
        } else if (hit_location == HIT_ABOVE) {
            auto shaft = shafts.upper_bound(pts[q]);
            above[q] = shaft == shafts.begin() ? -1 : prev(shaft)->second;
            below[q] = shaft == shafts.end() ? -1 : shaft->second;
        } else {
            assert(false && "Invalid hit location");
        }
    };

    for (int l = 0, r = 1; r <= N; l = r++) {
        int u = index[l];
        while (r < N && pts[u] == pts[index[r]]) {
            r++;
        }
        for (int i = l; i < r; i++) {
            for (int v = index[i], s = tail[v]; s != -1; s = next_tail[s]) {
                shafts.erase(rays[s]);
            }
        }
        for (int i = l; i < r; i++) {
            answer_query(index[i]);
        }
        for (int i = l; i < r; i++) {
            for (int v = index[i], s = head[v]; s != -1; s = next_head[s]) {
                shafts.emplace(rays[s], s);
            }
        }
    }

    return make_pair(move(below), move(above));
}

// Point q inside strict convex ccw polygon? -1=>outside, +1=>inside, 0=onedge. O(log n)
int strict_polygon_location(Pt2 q, const vector<Pt2>& poly) {
    int N = poly.size();
    assert(N >= 3);

    if (N == 3) {
        int a = orientation(poly[0], poly[1], q);
        int b = orientation(poly[1], poly[2], q);
        int c = orientation(poly[2], poly[0], q);
        return min({a, b, c});
    }

    int L = 1, R = N - 1;
    while (L + 1 < R) {
        int M = (L + R) / 2;
        auto cut = orientation(poly[0], poly[M], q);
        if (cut > 0) {
            L = M;
        } else if (cut < 0) {
            R = M;
        } else if (onsegment(poly[0], q, poly[M])) {
            return q == poly[0] || q == poly[M] ? 0 : +1;
        } else {
            return -1;
        }
    }

    int a = orientation(poly[0], poly[L], q);
    int b = orientation(poly[L], poly[R], q);
    int c = orientation(poly[R], poly[0], q);
    return min({a, b, c});
}
