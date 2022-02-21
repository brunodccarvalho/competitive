#pragma once

#include "geometry/geometry2d.hpp"

// Compute intersection of halfplanes as ordered list of halfplanes. O(n log n).
// All halfplanes point left (aka ccw of direction). Returns {} if intersection is empty.
// Suppresses null area contributors. In particular, returns nothing if the area is 0.
auto halfplane_isect(const vector<Ray>& hp) {
    int N = hp.size();
    vector<int> index(N);
    iota(begin(index), end(index), 0);
    sort(begin(index), end(index),
         [&](int i, int j) { return angle_sort(hp[i].d, hp[j].d); });

    // Filter consecutive halfplanes with the same direction, taking the most restrictive
    vector<int> relevant;
    for (int i = 0, R = relevant.size(); i < N; i++) {
        int u = index[i], v = R ? relevant.back() : -1;
        if (R == 0 || cross(hp[u].d, hp[v].d) || dot(hp[u].d, hp[v].d) <= 0) {
            relevant.push_back(u), R++;
        } else if (cross(hp[u].d, hp[v].p - hp[u].p) < 0) {
            relevant[R - 1] = u;
        }
    }

    auto empty_isect = [&](int u, int v, int w) {
        auto cuv = cross(hp[u].d, hp[v].d);
        auto cuw = cross(hp[u].d, hp[w].d);
        auto cvw = cross(hp[v].d, hp[w].d);
        return cuw < 0 ? cuv > 0 && cvw >= 0 && !hp[u].isect_compare_unsafe(hp[w], hp[v])
                       : cuw <= 0 && cross(hp[u].d, hp[w].p - hp[u].p) <= 0;
    };

    auto redundant = [&](int u, int v, int w) { // v is redundant between u and w
        return cross(hp[u].d, hp[w].d) > 0 && !hp[u].isect_compare_unsafe(hp[v], hp[w]);
    };

    deque<int> hull;
    int S = 0;

    for (int u : relevant) {
        while (S > 1) {
            if (redundant(hull[S - 2], hull[S - 1], u)) {
                hull.pop_back(), S--;
            } else if (redundant(u, hull[0], hull[1])) {
                hull.pop_front(), S--;
            } else if (empty_isect(hull[S - 2], hull[S - 1], u)) {
                return deque<int>();
            } else if (empty_isect(u, hull[0], hull[1])) {
                return deque<int>();
            } else {
                break;
            }
        }
        if (S <= 1 || !redundant(hull[S - 1], u, hull[0])) {
            hull.push_back(u), S++;
        }
    }

    while (S > 2) {
        if (redundant(hull[S - 2], hull[S - 1], hull[0])) {
            hull.pop_back(), S--;
        } else if (redundant(hull[S - 1], hull[0], hull[1])) {
            hull.pop_front(), S--;
        } else if (empty_isect(hull[S - 2], hull[S - 1], hull[0])) {
            return deque<int>();
        } else if (empty_isect(hull[S - 1], hull[0], hull[1])) {
            return deque<int>();
        } else {
            break;
        }
    }

    return hull;
}
