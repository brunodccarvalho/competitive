#pragma once

#include "geometry/geometry2d.hpp"
#include "algo/y_combinator.hpp"

auto extract_points(const vector<Pt2>& pts, const vector<int>& index) {
    int N = index.size();
    vector<Pt2> pick(N);
    for (int i = 0; i < N; i++) {
        pick[i] = pts[index[i]];
    }
    return pick;
}

// Merge two strictly ccw convex hulls, no restrictions. O(n)
auto merge_hulls(const vector<Pt2>& apts, const vector<Pt2>& bpts) {
    int A = apts.size(), B = bpts.size();
    if (A == 0 || B == 0) {
        return A ? apts : bpts;
    }

    auto* ap = apts.data();
    auto* bp = bpts.data();
    int a = min_element(ap, ap + A) - ap;
    int b = min_element(bp, bp + B) - bp;
    int on = bp[b] < ap[a];
    Pt2 first = on ? bp[b] : ap[a];

    // true if the hull should go to w instead of v
    auto turns = [&](const auto& u, const auto& v, const auto& w) {
        if (auto c = orientation(u, v, w)) {
            return c < 0;
        } else {
            return dot(v - u, w - v) >= 0;
        }
    };

    vector<Pt2> hull;
    do {
        if (on == 1) {
            swap(ap, bp), swap(a, b), swap(A, B);
        }
        int na = a + 1 < A ? a + 1 : 0;
        int nb = b + 1 < B ? b + 1 : 0;
        while (B > 1 && turns(ap[a], bp[b], bp[nb])) {
            b = nb, nb = b + 1 < B ? b + 1 : 0;
        }
        bool c = turns(ap[a], bp[b], ap[na]);
        a = na;
        if (c) {
            on = 0, hull.push_back(ap[a]);
        } else {
            on = 1, hull.push_back(bp[b]);
        }
    } while (hull.back() != first);

    return hull;
}

// Merge two strictly ccw convex hulls, no restrictions. O(n)
auto merge_hulls(const vector<Pt2>& pts, const vector<int>& ain, const vector<int>& bin) {
    int A = ain.size(), B = bin.size();
    if (A == 0 || B == 0) {
        return A ? ain : bin;
    }

    auto* ap = ain.data();
    auto* bp = bin.data();
    int a = 0, b = 0;
    for (int i = 1; i < A; i++)
        if (pts[ap[i]] < pts[ap[a]])
            a = i;
    for (int i = 1; i < B; i++)
        if (pts[bp[i]] < pts[bp[b]])
            b = i;

    int on = pts[bp[b]] < pts[ap[a]];
    Pt2 first = on ? pts[bp[b]] : pts[ap[a]];

    // true if v->w goes rightwards or backwards after u->v
    auto turns = [&](int u, int v, int w) {
        if (auto c = orientation(pts[u], pts[v], pts[w])) {
            return c < 0;
        } else {
            return dot(pts[v] - pts[u], pts[w] - pts[v]) >= 0;
        }
    };

    vector<int> hull;
    do {
        if (on == 1) {
            swap(ap, bp), swap(a, b), swap(A, B);
        }
        int na = a + 1 < A ? a + 1 : 0;
        int nb = b + 1 < B ? b + 1 : 0;
        while (B > 1 && turns(ap[a], bp[b], bp[nb])) {
            b = nb, nb = b + 1 < B ? b + 1 : 0;
        }
        bool c = turns(ap[a], bp[b], ap[na]);
        a = na;
        if (c) {
            on = 0, hull.push_back(ap[a]);
        } else {
            on = 1, hull.push_back(bp[b]);
        }
    } while (pts[hull.back()] != first);

    return hull;
}

// Compute strict ccw hull, no restrictions. O(n log n)
auto hull_monotone_chain(const vector<Pt2>& pts) {
    int N = pts.size();

    vector<int> index(N), upper, lower;
    iota(begin(index), end(index), 0);
    sort(begin(index), end(index), [&](int u, int v) { return pts[u] < pts[v]; });

    auto orient = [&](int u, int v, int w) {
        return orientation(pts[u], pts[v], pts[w]);
    };

    int i = 0, S = 0;
    while (i < N) { // lower hull, including lower-left and upper-right corners
        while (S > 1 && orient(lower[S - 2], lower[S - 1], index[i]) <= 0) {
            lower.pop_back(), S--;
        }
        lower.push_back(index[i++]), S++;
    }
    i = N - 1, S = 0;
    while (i >= 0) { // upper hull
        while (S > 1 && orient(upper[S - 2], upper[S - 1], index[i]) <= 0) {
            upper.pop_back(), S--;
        }
        upper.push_back(index[i--]), S++;
    }
    if (N > 1) {
        lower.pop_back();
    }
    if (N > 0) {
        upper.pop_back();
    }
    lower.insert(end(lower), begin(upper), end(upper));

    return lower;
}

auto hull_divide_conquer(const vector<Pt2>& pts) {
    return y_combinator([&](auto self, int L, int R) -> vector<int> {
        if (L + 2 == R && pts[L] != pts[L + 1]) {
            return {L, L + 1};
        } else if (L + 2 >= R) {
            return {L};
        } else {
            int M = L + (R - L) / 2;
            return merge_hulls(pts, self(L, M), self(M, R));
        }
    })(0, pts.size());
}

auto hull_divide_conquer_inplace(const vector<Pt2>& pts) {
    return y_combinator([&](auto self, int L, int R) -> vector<Pt2> {
        if (L + 2 == R && pts[L] != pts[L + 1]) {
            return {pts[L], pts[L + 1]};
        } else if (L + 2 >= R) {
            return {pts[L]};
        } else {
            int M = L + (R - L) / 2;
            return merge_hulls(self(L, M), self(M, R));
        }
    })(0, pts.size());
}
