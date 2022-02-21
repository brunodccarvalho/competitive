#pragma once

#include <bits/stdc++.h>
using namespace std;

__int128_t abs(__int128_t x) { return x >= 0 ? x : -x; }

// For 2D computational geometry problems

struct Pt2 {
    using T = int32_t;    // points, vectors, coefs, manh -- integer/frac/quot
    using L = int64_t;    // crosses, dotes, dist2, norm2 -- integer/frac/quot
    using H = __int128_t; // huge (circle predicates) -- int128/double
    static constexpr bool FLOAT = false;

    T x, y;
    Pt2() : x(0), y(0) {}
    Pt2(T x, T y) : x(x), y(y) {}

    auto paired() const { return make_pair(x, y); }
    friend bool operator==(Pt2 a, Pt2 b) { return a.paired() == b.paired(); }
    friend bool operator!=(Pt2 a, Pt2 b) { return a.paired() != b.paired(); }
    friend bool operator<(Pt2 a, Pt2 b) { return a.paired() < b.paired(); }
    friend bool operator>(Pt2 a, Pt2 b) { return a.paired() > b.paired(); }
    friend bool operator<=(Pt2 a, Pt2 b) { return a.paired() <= b.paired(); }
    friend bool operator>=(Pt2 a, Pt2 b) { return a.paired() >= b.paired(); }

    explicit operator bool() const noexcept { return x || y; }

    bool boxed(Pt2 lo, Pt2 hi) const {
        return lo.x <= x && x <= hi.x && lo.y <= y && y <= hi.y;
    }
    friend auto min(Pt2 a, Pt2 b) { return Pt2(min(a.x, b.x), min(a.y, b.y)); }
    friend auto max(Pt2 a, Pt2 b) { return Pt2(max(a.x, b.x), max(a.y, b.y)); }

    T& operator[](int i) { return *(&x + i); }
    T operator[](int i) const { return *(&x + i); }
    Pt2 operator-() const { return Pt2(-x, -y); }
    Pt2 operator+() const { return Pt2(x, y); }
    friend Pt2 operator+(Pt2 u, Pt2 v) { return Pt2(u.x + v.x, u.y + v.y); }
    friend Pt2 operator-(Pt2 u, Pt2 v) { return Pt2(u.x - v.x, u.y - v.y); }
    friend Pt2 operator*(T k, Pt2 u) { return Pt2(u.x * k, u.y * k); }
    friend Pt2 operator*(Pt2 u, T k) { return Pt2(u.x * k, u.y * k); }
    friend Pt2 operator/(Pt2 u, T k) { return Pt2(u.x / k, u.y / k); }
    friend Pt2& operator+=(Pt2& u, Pt2 v) { return u = u + v; }
    friend Pt2& operator-=(Pt2& u, Pt2 v) { return u = u - v; }
    friend Pt2& operator*=(Pt2& u, T k) { return u = u * k; }
    friend Pt2& operator/=(Pt2& u, T k) { return u = u / k; }

    friend auto refl(Pt2 u) { return Pt2(u.y, u.x); }  // reflection across xOy bisector
    friend auto conj(Pt2 u) { return Pt2(u.x, -u.y); } // complex conjugate
    friend auto perp_ccw(Pt2 u) { return Pt2(-u.y, u.x); } // rh rotation
    friend auto perp_cw(Pt2 u) { return Pt2(u.y, -u.x); }  // lh rotation

    friend auto manh(Pt2 u) { return abs(u.x) + abs(u.y); }
    friend auto manh(Pt2 a, Pt2 b) { return manh(a - b); }
    friend auto abs(Pt2 u) { return Pt2(abs(u.x), abs(u.y)); }

    friend auto dot(Pt2 u, Pt2 v) { return L(u.x) * v.x + L(u.y) * v.y; }
    friend auto norm2(Pt2 u) { return dot(u, u); }
    friend auto norm(Pt2 u) { return std::sqrt(double(norm2(u))); }

    friend auto cross(Pt2 u, Pt2 v) { return L(u.x) * v.y - L(u.y) * v.x; }
    friend auto cross(Pt2 a, Pt2 b, Pt2 c) { return cross(b - a, c - a); }

    friend auto dist2(Pt2 a, Pt2 b) { return norm2(a - b); }
    friend auto dist(Pt2 a, Pt2 b) { return norm(a - b); }

    friend auto int_norm(Pt2 u) { return abs(gcd(u.x, u.y)); }
    friend auto int_unit(Pt2 u) { return u / int_norm(u); }

    friend string to_string(Pt2 p) {
        return '(' + to_string(p.x) + ',' + to_string(p.y) + ')';
    }
    friend ostream& operator<<(ostream& out, Pt2 p) { return out << to_string(p); }
    friend istream& operator>>(istream& in, Pt2& p) { return in >> p.x >> p.y; }

    friend int orientation(Pt2 a, Pt2 b, Pt2 c) {
        auto sign = cross(a, b, c);
        return (sign > 0) - (sign < 0);
    }

    friend bool collinear(Pt2 a, Pt2 b, Pt2 c) { return orientation(a, b, c) == 0; }
    friend bool samedir(Pt2 u, Pt2 v) { return cross(u, v) == 0; }
    friend bool onsegment(Pt2 a, Pt2 b, Pt2 c) {
        return orientation(a, b, c) == 0 && dot(a - b, c - b) <= 0;
    }

    friend auto signed_linedist(Pt2 a, Pt2 u, Pt2 v) {
        return double(cross(a, u, v)) / dist(u, v);
    }
    friend auto linedist(Pt2 a, Pt2 u, Pt2 v) { return abs(signed_linedist(a, u, v)); }
    friend auto raydist(Pt2 a, Pt2 u, Pt2 v) {
        if (dot(a - u, v - u) <= 0) {
            return dist(a, u);
        } else {
            return linedist(a, u, v);
        }
    }
    friend auto segdist(Pt2 a, Pt2 u, Pt2 v) {
        if (dot(a - u, v - u) <= 0) {
            return dist(a, u);
        } else if (dot(a - v, u - v) <= 0) {
            return dist(a, v);
        } else {
            return linedist(a, u, v);
        }
    }

    friend auto cos(Pt2 u) { return clamp<double>(double(u.x) / norm(u), -1.0, 1.0); }
    friend auto sin(Pt2 u) { return clamp<double>(double(u.y) / norm(u), -1.0, 1.0); }
    friend auto angle(Pt2 u) {
        static const double TAU = 4 * acos(0);
        auto ang = std::atan2(u.y, u.x);
        return ang < 0 ? ang + TAU : ang;
    }

    friend auto cos(Pt2 u, Pt2 v) {
        return clamp<double>(double(dot(u, v)) / (norm(u) * norm(v)), -1.0, 1.0);
    }
    friend auto sin(Pt2 u, Pt2 v) {
        return clamp<double>(double(cross(u, v)) / (norm(u) * norm(v)), -1.0, 1.0);
    }
    friend auto ccw_angle(Pt2 u, Pt2 v) {
        static const double TAU = 4 * acos(0);
        auto ang = std::atan2(cross(u, v), dot(u, v));
        return ang < 0 ? ang + TAU : ang;
    }
};

int quadrant(Pt2 u) {
    if (u.x > 0 && u.y >= 0) {
        return 1;
    } else if (u.x <= 0 && u.y > 0) {
        return 2;
    } else if (u.x < 0 && u.y <= 0) {
        return 3;
    } else if (u.x >= 0 && u.y < 0) {
        return 4;
    } else {
        return 0;
    }
}

int directed_quadrant(Pt2 u, Pt2 forward) {
    auto x = dot(forward, u), y = cross(forward, u);
    if (x > 0 && y >= 0) {
        return 1;
    } else if (x <= 0 && y > 0) {
        return 2;
    } else if (x < 0 && y <= 0) {
        return 3;
    } else if (x >= 0 && y < 0) {
        return 4;
    } else {
        return 0;
    }
}

bool angle_sort(Pt2 u, Pt2 v) {
    int qu = quadrant(u), qv = quadrant(v);
    if (qu != qv) {
        return qu < qv;
    } else if (auto cuv = cross(u, v)) {
        return cuv > 0;
    } else {
        return norm2(u) < norm2(v);
    }
}

bool biangle_sort(Pt2 u, Pt2 v) {
    return angle_sort(quadrant(u) < 3 ? u : -u, quadrant(v) < 3 ? v : -v);
}

auto angle_sorter(Pt2 pivot, Pt2 forward = Pt2(1, 0)) {
    assert(forward);
    return [=](Pt2 u, Pt2 v) -> bool {
        u -= pivot, v -= pivot;
        int qu = directed_quadrant(u, forward), qv = directed_quadrant(v, forward);
        if (qu != qv) {
            return qu < qv;
        } else if (auto cuv = cross(u, v)) {
            return cuv > 0;
        } else {
            return norm2(u) < norm2(v);
        }
    };
}

auto biangle_sorter(Pt2 pivot, Pt2 forward = Pt2(1, 0)) {
    assert(forward);
    return [=](Pt2 u, Pt2 v) -> bool {
        u -= pivot, v -= pivot;
        int qu = directed_quadrant(u, forward), qv = directed_quadrant(v, forward);
        if (qu >= 3)
            qu -= 2, u = -u;
        if (qv >= 3)
            qv -= 2, v = -v;
        if (qu != qv) {
            return qu < qv;
        } else if (auto cuv = cross(u, v)) {
            return cuv > 0;
        } else {
            return norm2(u) < norm2(v);
        }
    };
}

auto line_sorter(Pt2 along = Pt2(1, 0)) {
    assert(along);
    return [=](Pt2 u, Pt2 v) -> bool {
        if (auto duv = dot(v - u, along)) {
            return duv > 0;
        } else {
            return cross(v - u, along) < 0;
        }
    };
}

struct Ray {
    Pt2 p, d; // p + dt

    static Ray ray(Pt2 point, Pt2 dt) { return Ray{point, dt}; }
    static Ray slope(Pt2 dt) { return Ray{Pt2(), dt}; }
    static Ray through(Pt2 u, Pt2 v) { return Ray{u, v - u}; }
    static Ray halfplane(Pt2 point, Pt2 normal) { return Ray{point, perp_cw(normal)}; }

    auto operator-() const { return Ray{p, -d}; }
    auto q() const { return p + d; }

    friend auto signed_linedist(Pt2 u, Ray l) { return signed_linedist(u, l.p, l.q()); }
    friend auto linedist(Pt2 u, Ray l) { return linedist(u, l.p, l.q()); }
    friend auto raydist(Pt2 u, Ray l) { return raydist(u, l.p, l.q()); }
    friend auto segdist(Pt2 u, Ray l) { return segdist(u, l.p, l.q()); }
    friend int orientation(Pt2 u, Ray l) { return orientation(l.p, l.q(), u); }

    // +2=> left; +1=> <=line.p on ray; 0=> on segment; -1=> >=line.q on ray; -2=> right
    friend int fiveway_orientation(Pt2 u, const Ray& line, int positive_side = +1) {
        if (int side = orientation(u, line)) {
            return positive_side > 0 ? 2 * side : -2 * side;
        } else {
            auto dup = dot(line.d, line.p - u), dqu = dot(line.d, u - line.q());
            return (dup >= 0) - (dqu >= 0);
        }
    }

    // Which line, a or b, intersects l first? Return true if a intersects first
    // If a or b is parallel to l, the answer is always false
    bool isect_compare_unsafe(Ray a, Ray b) const {
        // a hits at (an/ad)l.d, while b hits at (bn/bd)l.d
        auto an = cross(a.p - p, a.d), ad = cross(d, a.d);
        auto bn = cross(b.p - p, b.d), bd = cross(d, b.d);
        auto diff = an * bd - bn * ad;
        return (ad > 0) == (bd > 0) ? diff < 0 : diff > 0;
    }

    // Let w be the projection of u on this line, find k such that w=p+kd
    double coef(Pt2 u) const { return double(dot(u - p, d)) / norm2(d); }
};

struct Circle {
    static inline const double TAU = 4 * acos(0);

    Pt2 center;
    Pt2::L r2;

    static Circle around2(Pt2 center, Pt2::L r2) { return Circle{center, r2}; }
    static Circle around(Pt2 center, Pt2::T r) { return Circle{center, Pt2::L(r) * r}; }

    auto radius() const { return std::sqrt(double(r2)); }
    auto power(Pt2 u) const { return dist2(u, center) - r2; }
    auto circledist(Pt2 u) const { return dist(u, center) - radius(); }

    int circleside(Pt2 u) const {
        auto p = power(u);
        return (p > 0) - (p < 0);
    }
    int circleside(Ray ray) const {
        auto d = linedist(center, ray);
        return (d > radius()) - (d < radius());
    }

    auto tangent_slopes(Pt2 u) const {
        auto A = angle(u - center);
        auto B = acos(cos(sqrt(power(u) / dist2(u, center))));
        A += A < 0 ? TAU : 0;
        return make_pair(A - B, A + B);
    }
};

bool segments_intersect(Pt2 a, Pt2 b, Pt2 u, Pt2 v, bool touching_counts = true,
                        bool holding_counts = false) {
    // Doing this correctly is so fucking annoying
    assert(a != b && u != v);
    auto uva = orientation(u, v, a), uvb = orientation(u, v, b);
    auto abu = orientation(a, b, u), abv = orientation(a, b, v);
    if (uva && uva == uvb)
        return false; // ab lies strictly to one side of uv
    if (abu && abu == abv)
        return false; // uv lies strictly to one side of ab
    if (u == a)
        return holding_counts || (abv == 0 && uvb == 0 && !onsegment(b, a, v));
    if (a == v)
        return holding_counts || (abu == 0 && uvb == 0 && !onsegment(b, a, u));
    if (b == u)
        return holding_counts || (abv == 0 && uva == 0 && !onsegment(a, b, v));
    if (b == v)
        return holding_counts || (abu == 0 && uva == 0 && !onsegment(a, b, u));
    if (onsegment(a, u, b))
        return touching_counts || collinear(a, u, v) || collinear(b, u, v);
    if (onsegment(a, v, b))
        return touching_counts || collinear(a, u, v) || collinear(b, u, v);
    if (onsegment(u, a, v))
        return touching_counts || collinear(u, a, b) || collinear(v, a, b);
    if (onsegment(u, b, v))
        return touching_counts || collinear(u, a, b) || collinear(v, a, b);
    return uva != uvb && abu != abv;
}
