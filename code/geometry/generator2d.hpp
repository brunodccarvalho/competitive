#pragma once

#include "random.hpp"
#include "geometry/geometry2d.hpp"
#include "geometry/utils2d.hpp"
#include "numeric/quot.hpp"

using Pointset = std::unordered_set<array<int64_t, 2>>;

auto floatint_point(double x, double y) {
    if constexpr (Pt2::FLOAT) {
        return Pt2(x, y);
    } else {
        return Pt2(llround(x), llround(y));
    }
}

auto floatint_insert_pointset(Pointset& pointset, Pt2 point) {
    static constexpr double MULT = 1e5;
    static constexpr array<array<int, 2>, 4> ddd = {{{0, 0}, {0, 1}, {1, 0}, {1, 1}}};
    if constexpr (Pt2::FLOAT) {
        int64_t x = floor(point.x * MULT);
        int64_t y = floor(point.y * MULT);
        bool ok = true;
        for (auto [dx, dy] : ddd) {
            ok &= !pointset.count({x + dx, y + dy});
        }
        if (ok) {
            for (auto [dx, dy] : ddd) {
                pointset.insert({x + dx, y + dy});
            }
        }
        return ok;
    } else {
        int64_t x = point.x, y = point.y;
        return pointset.insert({x, y}).second;
    }
}

auto floatint_modulo_point(Pt2::T x, Pt2::T y, Pt2::T mod) {
    if constexpr (Pt2::FLOAT) {
        return Pt2(fmod(x, mod), fmod(y, mod));
    } else {
        return Pt2(x % mod, y % mod);
    }
}

enum class PointDistrib {
    SQUARE,          // interior of an axis-aligned square
    SQUARE_EDGES,    // edges of an axis-aligned square (~8R points)
    DISK,            // interior of a circle
    CIRCLE,          // curve of a circle (rounded for ints) (~8R points)
    PARABOLA,        // parabola above x (~2R points)
    DOUBLE_PARABOLA, // parabola above and below x (~4R points)
    STARLINES,       // points on axis and quadrant bisectors (~8R points)
    QUAD_MODULO,     // points (x,(2x²+1)mod Pt2) for a suitably chosen prime Pt2
    END,
};

string to_string(PointDistrib distr) {
    static const string names[] = {
        "square"s,          "square-edges"s, "disk"s,          "circle"s, "parabola"s,
        "double-parabola"s, "starlines"s,    "linear-modulo"s, "END"s,
    };
    return names[int(distr)];
}

auto rand_point_distribution() {
    static discrete_distribution<int> dist({3, 3, 3, 5, 4, 4, 2, 3});
    return PointDistrib(dist(mt));
}

void rotate_coordinates(vector<Pt2>& pts) {
    for (auto& p : pts) {
        p = Pt2(p.y, p.x);
    }
}

auto rand_square(Pt2::T R = 2000) {
    auto x = rand_unif<Pt2::T>(-R, R);
    auto y = rand_unif<Pt2::T>(-R, R);
    return Pt2(x, y);
}

auto rand_square_edges(Pt2::T R = 2000) {
    static boold coind(0.5);
    auto x = rand_unif<Pt2::T>(-R, R);
    auto y = rand_unif<Pt2::T>(-R, R);
    Pt2 p(x, y);
    p[rand_unif<int>(0, 1)] = coind(mt) ? R : -R;
    return p;
}

auto rand_disk(Pt2::T R = 2000, int repulsion = 1) {
    static normald dist(0, 1);
    auto x = dist(mt), y = dist(mt);
    auto n = sqrt(x * x + y * y);
    auto r = rand_wide<double>(0, R, repulsion);
    return floatint_point(r * x / n, r * y / n);
}

auto rand_circle(Pt2::T R = 2000) {
    static normald dist(0, 1);
    auto x = dist(mt), y = dist(mt);
    auto n = sqrt(x * x + y * y);
    return floatint_point(R * x / n, R * y / n);
}

auto rand_parabola(Pt2::T R = 2000, int gravity = -1) {
    auto x = rand_grav<Pt2::T>(-R, R, gravity);
    return Pt2(x, x * x / R);
}

auto rand_double_parabola(Pt2::T R = 2000, int gravity = -1) {
    static boold coind(0.5);
    auto x = rand_grav<Pt2::T>(-R, R, gravity);
    return Pt2(x, coind(mt) ? x * x / R : -x * x / R);
}

auto rand_starlines(Pt2::T R = 2000, int gravity = -1) {
    static discrete_distribution sided({3, 2, 3});
    int a, b;
    do {
        a = sided(mt) - 1, b = sided(mt) - 1;
    } while (a == 0 && b == 0);
    auto x = rand_grav<Pt2::T>(-R, R, gravity);
    return Pt2(a * x, b * x);
}

auto rand_quad_modulo(Pt2::T R = 2000, int gravity = -1) {
    static constexpr int primes[17] = {7,          19,        37,         73,
                                       139,        491,       991,        2971,
                                       9967,       39'983,    99'989,     399'983,
                                       999'979,    9'999'973, 49'999'921, 149'999'909,
                                       499'999'931};
    int i = upper_bound(primes, primes + 17, R) - primes - 1;
    Pt2::L x = rand_grav<Pt2::T>(1, R, gravity);
    return floatint_modulo_point(x, 2 * x * x + 1, primes[i]);
}

auto add_collinear_between(int N, Pointset& pointset, vector<Pt2>& pts) {
    int S = pts.size();
    int runs = 100 * N;
    assert(N == 0 || S > 1);
    while (N && runs--) {
        auto [i, j] = different<int>(0, S - 1);
        auto uv = pts[j] - pts[i];
        Pt2 point;

        if constexpr (Pt2::FLOAT) {
            constexpr double EPS = 1e-5;
            auto a = rand_unif<int>(0 + EPS, 1 - EPS);
            point = pts[i] + a * uv;
        } else {
            auto g = abs(gcd(uv.x, uv.y));
            auto m = rand_unif<Pt2::T>(0, g);
            point = pts[i] + uv * m / g;
        }

        if (floatint_insert_pointset(pointset, point)) {
            N--, pts.push_back(point), S++;
        }
    }
}

auto rand_rotrefl(vector<Pt2>& pts) {
    static boold coind(0.5);
    for (int i = 0; i <= 1; i++) {
        if (coind(mt)) {
            for (auto& pt : pts) {
                pt[i] = -pt[i];
            }
        }
    }
    if (coind(mt)) {
        for (auto& pt : pts) {
            pt = perp_ccw(pt);
        }
    }
}

auto generate_point(PointDistrib distr, Pt2::T R = 2000) {
    switch (distr) {
    case PointDistrib::SQUARE:
        return rand_square(R);
    case PointDistrib::SQUARE_EDGES:
        return rand_square_edges(R);
    case PointDistrib::DISK:
        return rand_disk(R);
    case PointDistrib::CIRCLE:
        return rand_circle(R);
    case PointDistrib::PARABOLA:
        return rand_parabola(R);
    case PointDistrib::DOUBLE_PARABOLA:
        return rand_double_parabola(R);
    case PointDistrib::STARLINES:
        return rand_starlines(R);
    case PointDistrib::QUAD_MODULO:
        return rand_quad_modulo(R);
    default:
        throw runtime_error("Invalid distribution");
    }
}

// Generate N points, no two coincident
auto generate_points(int N, PointDistrib distr, int L = 0, Pt2::T R = 2000) {
    Pointset pointset;
    vector<Pt2> pts;
    while (N) {
        auto point = generate_point(distr, R);
        if (floatint_insert_pointset(pointset, point)) {
            N--, pts.push_back(point);
        }
    }
    add_collinear_between(L, pointset, pts);
    rand_rotrefl(pts);
    shuffle(begin(pts), end(pts), mt);
    assert(bounding_box_size(pts).boxed(Pt2(), 3 * Pt2(R, R)));
    return pts;
}

// Given some points, return a random set of at most S non-intersecting segments. The
// algorithm may fail to produce S segments, in which case it returns fewer.
// - touching: if true, allow a segment's endpoint to be in the middle of another segment
// Somewhere between O(SN log N) and O(N^2 log N) on average
// You can provide some segments or points or act as blockers
auto non_overlapping_sample(const vector<Pt2>& pts, int S,
                            const vector<pair<Pt2, Pt2>>& blockers,
                            bool touching = false) {
    int N = pts.size();
    vector<array<int, 2>> segments;
    S = N >= 3 ? min(S, 3 * N - 6) : min(S, 1);

    // Just pick a segment uniformly at random initially, being optimistic
    // Go through with ~10(n choose 2) runs
    long runs = 5LL * N * N;
    while (runs-- && S > 0) {
        auto [u, v] = different<int>(0, N - 1);
        bool accept = pts[u] != pts[v];
        for (int i = 0, B = segments.size(); accept && i < B; i++) {
            auto [a, b] = segments[i];
            accept &= !segments_intersect(pts[u], pts[v], pts[a], pts[b], !touching);
        }
        for (int i = 0, B = blockers.size(); accept && i < B; i++) {
            auto [a, b] = blockers[i];
            accept &= a == b ? !onsegment(pts[u], a, pts[v])
                             : !segments_intersect(pts[u], pts[v], a, b, !touching);
        }
        for (int i = 0; accept && i < N && !touching; i++) {
            accept &= i == u || i == v || !onsegment(pts[u], pts[i], pts[v]);
        }
        if (accept) {
            segments.push_back({u, v}), S--;
        }
    }

    shuffle(begin(segments), end(segments), mt);
    return segments;
}

// Given a parameterless point generator, pick S non-crossing, non-holding and
// non-touching segments. Quadratic in the number of segments on average. You may give
// some segments up front to act as blockers.
template <typename PGen>
auto non_holding_sample(PGen&& gen, int S, const vector<array<Pt2, 2>>& blockers = {}) {
    vector<array<Pt2, 2>> segments;
    int N = 0, B = blockers.size();

    // Just pick a segment uniformly at random initially, being optimistic
    // Go through with ~10(n choose 2) runs
    while (N < S) {
        Pt2 u = gen(), v = gen();
        bool accept = u != v;
        for (int i = 0; i < B && accept; i++) {
            auto [a, b] = blockers[i];
            if (a == b) {
                accept &= !onsegment(u, a, v);
            } else {
                accept &= !segments_intersect(u, v, a, b, true, true);
            }
        }
        for (int i = 0; i < N && accept; i++) {
            auto [a, b] = segments[i];
            accept &= !segments_intersect(u, v, a, b, true, true);
        }
        if (accept) {
            segments.push_back({u, v}), N++;
        }
    }

    shuffle(begin(segments), end(segments), mt);
    return segments;
}

// Verify if any triple of points are collinear. O(n^2 log n) and O(n) memory
auto find_three_collinear(const vector<Pt2>& pts) {
    int N = pts.size();
    if (N <= 2) {
        return make_tuple(-1, -1, -1);
    }

    vector<int> order(N);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order), [&](int u, int v) { return pts[u] < pts[v]; });

    vector<int> rank(N);
    for (int i = 0; i < N; i++) {
        rank[order[i]] = i;
    }

    for (int i = 1; i < N; i++) {
        if (pts[order[i]] == pts[order[i - 1]]) {
            return make_tuple(order[i], order[i - 1], i > 1 ? 0 : N - 1);
        }
    }
    for (int i = 2; i < N; i++) {
        if (collinear(pts[order[i - 2]], pts[order[i - 1]], pts[order[i]])) {
            return make_tuple(order[i - 2], order[i - 1], order[i]);
        }
    }

    using Q = quot<Pt2::L, true>;

    auto get_slope = [&](int u, int v) {
        if (pts[u].x == pts[v].x) {
            return Q(1, 0);
        } else {
            return Q(pts[u].y - pts[v].y, pts[u].x - pts[v].x);
        }
    };

    set<pair<Q, int>> events;
    vector<set<pair<Q, int>>::iterator> its(N);

    auto insert_slope = [&](int i) {
        if (pts[order[i - 1]].x < pts[order[i]].x) {
            its[i] = events.emplace(get_slope(order[i - 1], order[i]), i).first;
        }
    };

    for (int i = 1; i < N; i++) {
        insert_slope(i);
    }

    while (events.size()) {
        auto [s, i] = *events.begin();
        events.erase(events.begin());

        int u = order[i - 1], v = order[i];
        int a = i > 1 ? order[i - 2] : -1;
        int b = i + 1 < N ? order[i + 1] : -1;
        if (a != -1 && collinear(pts[u], pts[v], pts[a])) {
            return make_tuple(a, u, v);
        }
        if (b != -1 && collinear(pts[u], pts[v], pts[b])) {
            return make_tuple(u, v, b);
        }

        if (i > 1) {
            events.erase(its[i - 1]);
        }
        if (i + 1 < N) {
            events.erase(its[i + 1]);
        }

        swap(rank[u], rank[v]);
        swap(order[i - 1], order[i]);

        if (i > 1) {
            insert_slope(i - 1);
        }
        if (i + 1 < N) {
            insert_slope(i + 1);
        }
        insert_slope(i);
    }

    return make_tuple(-1, -1, -1);
}
