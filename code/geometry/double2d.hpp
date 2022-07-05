#pragma once

#include <bits/stdc++.h>
using namespace std;

struct Pd2 {
    using T = double;
    using L = double;
    using H = double;
    static constexpr bool FLOAT = true;
    static constexpr double inf = numeric_limits<double>::infinity();

    T x, y;
    Pd2() : x(0), y(0) {}
    Pd2(T x, T y) : x(x), y(y) {}
    template <typename A>
    Pd2(A point) : x(point[0]), y(point[1]) {}

    auto paired() const { return make_pair(x, y); }
    friend bool operator==(Pd2 a, Pd2 b) { return a.paired() == b.paired(); }
    friend bool operator!=(Pd2 a, Pd2 b) { return a.paired() != b.paired(); }
    friend bool operator<(Pd2 a, Pd2 b) { return a.paired() < b.paired(); }
    friend bool operator>(Pd2 a, Pd2 b) { return a.paired() > b.paired(); }

    explicit operator bool() const noexcept { return x || y; }

    bool boxed(Pd2 lo, Pd2 hi) const {
        return lo.x <= x && x <= hi.x && lo.y <= y && y <= hi.y;
    }
    friend auto min(Pd2 a, Pd2 b) { return Pd2(min(a.x, b.x), min(a.y, b.y)); }
    friend auto max(Pd2 a, Pd2 b) { return Pd2(max(a.x, b.x), max(a.y, b.y)); }

    T& operator[](int i) { return *(&x + i); }
    T operator[](int i) const { return *(&x + i); }
    Pd2 operator-() const { return Pd2(-x, -y); }
    Pd2 operator+() const { return Pd2(x, y); }
    friend Pd2 operator+(Pd2 u, Pd2 v) { return Pd2(u.x + v.x, u.y + v.y); }
    friend Pd2 operator-(Pd2 u, Pd2 v) { return Pd2(u.x - v.x, u.y - v.y); }
    friend Pd2 operator*(T k, Pd2 u) { return Pd2(u.x * k, u.y * k); }
    friend Pd2 operator*(Pd2 u, T k) { return Pd2(u.x * k, u.y * k); }
    friend Pd2 operator/(Pd2 u, T k) { return Pd2(u.x / k, u.y / k); }
    friend Pd2& operator+=(Pd2& u, Pd2 v) { return u = u + v; }
    friend Pd2& operator-=(Pd2& u, Pd2 v) { return u = u - v; }
    friend Pd2& operator*=(Pd2& u, T k) { return u = u * k; }
    friend Pd2& operator/=(Pd2& u, T k) { return u = u / k; }

    friend auto refl(Pd2 u) { return Pd2(u.y, u.x); }  // reflection across xOy bisector
    friend auto conj(Pd2 u) { return Pd2(u.x, -u.y); } // complex conjugate
    friend auto perp_ccw(Pd2 u) { return Pd2(-u.y, u.x); } // rh rotation
    friend auto perp_cw(Pd2 u) { return Pd2(u.y, -u.x); }  // lh rotation

    friend auto manh(Pd2 u) { return abs(u.x) + abs(u.y); }
    friend auto manh(Pd2 a, Pd2 b) { return manh(a - b); }
    friend auto abs(Pd2 u) { return Pd2(abs(u.x), abs(u.y)); }

    friend auto dot(Pd2 u, Pd2 v) { return u.x * v.x + u.y * v.y; }
    friend auto norm2(Pd2 u) { return dot(u, u); }
    friend auto norm(Pd2 u) { return std::sqrt(norm2(u)); }

    friend auto cross(Pd2 u, Pd2 v) { return u.x * v.y - u.y * v.x; }
    friend auto cross(Pd2 a, Pd2 b, Pd2 c) { return cross(b - a, c - a); }

    friend auto dist2(Pd2 a, Pd2 b) { return norm2(a - b); }
    friend auto dist(Pd2 a, Pd2 b) { return norm(a - b); }
    friend auto dot_cross(Pd2 a, Pd2 b) { return Pd2(cross(a, b), dot(a, b)); }

    friend auto unit(Pd2 u) { return u / norm(u); }

    friend auto rotate(Pd2 u, double rad) {
        return dot_cross(u, Pd2(std::sin(rad), std::cos(rad)));
    }

    friend string to_string(Pd2 p) {
        return '(' + to_string(p.x) + ',' + to_string(p.y) + ')';
    }
    friend ostream& operator<<(ostream& out, Pd2 p) { return out << to_string(p); }
    friend istream& operator>>(istream& in, Pd2& p) { return in >> p.x >> p.y; }

    friend int orientation(Pd2 a, Pd2 b, Pd2 c) {
        // To return ±1 the crosses must all agree on sign and magnitude.
        auto va = cross(a, b, c), vb = cross(b, c, a), vc = cross(c, a, b);
        int sa = (va > 0) - (va < 0), sb = (vb > 0) - (vb < 0), sc = (vc > 0) - (vc < 0);
        auto sum = abs(va + vb + vc), dif = max({va, vb, vc}) - min({va, vb, vc});
        return sa == sb && sb == sc && dif <= 1e-5 * sum ? sa : 0;
    }

    friend bool collinear(Pd2 a, Pd2 b, Pd2 c) { return orientation(a, b, c) == 0; }
    friend bool samedir(Pd2 u, Pd2 v) { return orientation(Pd2(), u, v) == 0; }
    friend bool onsegment(Pd2 a, Pd2 b, Pd2 c) {
        return orientation(a, b, c) == 0 && dot(a - b, c - b) <= 0;
    }

    friend auto signed_linedist(Pd2 a, Pd2 u, Pd2 v) {
        return double(cross(a, u, v)) / dist(u, v);
    }
    friend auto linedist(Pd2 a, Pd2 u, Pd2 v) { return abs(signed_linedist(a, u, v)); }
    friend auto raydist(Pd2 a, Pd2 u, Pd2 v) {
        if (dot(a - u, v - u) <= 0) {
            return dist(a, u);
        } else {
            return linedist(a, u, v);
        }
    }
    friend auto segdist(Pd2 a, Pd2 u, Pd2 v) {
        if (dot(a - u, v - u) <= 0) {
            return dist(a, u);
        } else if (dot(a - v, u - v) <= 0) {
            return dist(a, v);
        } else {
            return linedist(a, u, v);
        }
    }

    friend auto cos(Pd2 u) { return clamp<double>(u.x / norm(u), -1.0, 1.0); }
    friend auto sin(Pd2 u) { return clamp<double>(u.y / norm(u), -1.0, 1.0); }
    friend auto angle(Pd2 u) {
        static const double TAU = 4 * acos(0);
        auto ang = std::atan2(u.y, u.x);
        return ang < 0 ? ang + TAU : ang;
    }

    friend auto cos(Pd2 u, Pd2 v) {
        return clamp<double>(dot(u, v) / (norm(u) * norm(v)), -1.0, 1.0);
    }
    friend auto sin(Pd2 u, Pd2 v) {
        return clamp<double>(cross(u, v) / (norm(u) * norm(v)), -1.0, 1.0);
    }
    friend auto ccw_angle(Pd2 u, Pd2 v) {
        static const double TAU = 4 * acos(0);
        auto ang = std::atan2(cross(u, v), dot(u, v));
        return ang < 0 ? ang + TAU : ang;
    }

    friend int quadrant(Pd2 u) {
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

    friend int directed_quadrant(Pd2 u, Pd2 forward) {
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

    friend bool angle_sort(Pd2 u, Pd2 v) {
        int qu = quadrant(u), qv = quadrant(v);
        if (qu != qv) {
            return qu < qv;
        } else if (auto cuv = angle(v) - angle(u)) {
            return cuv > 0;
        } else {
            return norm2(u) < norm2(v);
        }
    }

    friend bool biangle_sort(Pd2 u, Pd2 v) {
        return angle_sort(quadrant(u) < 3 ? u : -u, quadrant(v) < 3 ? v : -v);
    }

    friend auto angle_sorter(Pd2 pivot, Pd2 forward = Pd2(1, 0)) {
        assert(forward != Pd2());
        return [=](Pd2 u, Pd2 v) -> bool {
            u -= pivot, v -= pivot;
            int qu = directed_quadrant(u, forward), qv = directed_quadrant(v, forward);
            if (qu != qv) {
                return qu < qv;
            } else if (auto cuv = angle(v) - angle(u)) {
                return cuv > 0;
            } else {
                return norm2(u) < norm2(v);
            }
        };
    }

    friend auto biangle_sorter(Pd2 pivot, Pd2 forward = Pd2(1, 0)) {
        assert(forward != Pd2());
        return [=](Pd2 u, Pd2 v) -> bool {
            u -= pivot, v -= pivot;
            int qu = directed_quadrant(u, forward), qv = directed_quadrant(v, forward);
            if (qu >= 3)
                qu -= 2, u = -u;
            if (qv >= 3)
                qv -= 2, v = -v;
            if (qu != qv) {
                return qu < qv;
            } else if (auto cuv = angle(v) - angle(u)) {
                return cuv > 0;
            } else {
                return norm2(u) < norm2(v);
            }
        };
    }

    friend auto line_sorter(Pd2 along = Pd2(1, 0)) {
        assert(along != Pd2());
        return [=](Pd2 u, Pd2 v) -> bool {
            if (auto duv = dot(along, v) - dot(along, u)) {
                return duv > 0;
            } else {
                return dot(perp_ccw(along), v) - dot(perp_ccw(along), u) > 0;
            }
        };
    }
};

struct Rayd {
    Pd2 p, d; // p + dt

    static Rayd ray(Pd2 point, Pd2 dt) { return Rayd{point, dt}; }
    static Rayd slope(Pd2 dt) { return Rayd{Pd2(), dt}; }
    static Rayd through(Pd2 u, Pd2 v) { return Rayd{u, v - u}; }
    static Rayd halfplane(Pd2 point, Pd2 normal) { return Rayd{point, perp_cw(normal)}; }

    auto operator-() const { return Rayd{p, -d}; }
    auto q() const { return p + d; }

    friend auto signed_linedist(Pd2 u, Rayd l) { return signed_linedist(u, l.p, l.q()); }
    friend auto linedist(Pd2 u, Rayd l) { return linedist(u, l.p, l.q()); }
    friend auto raydist(Pd2 u, Rayd l) { return raydist(u, l.p, l.q()); }
    friend auto segdist(Pd2 u, Rayd l) { return segdist(u, l.p, l.q()); }
    friend int orientation(Pd2 u, Rayd l) { return orientation(l.p, l.q(), u); }

    // +2=> left; +1=> <=line.p on ray; 0=> on segment; -1=> >=line.q on ray; -2=> right
    friend int fiveway_orientation(Pd2 u, const Rayd& line, int positive_side = +1) {
        if (int side = orientation(u, line)) {
            return positive_side > 0 ? 2 * side : -2 * side;
        } else {
            auto dup = dot(line.d, line.p - u), dqu = dot(line.d, u - line.q());
            return (dup >= 0) - (dqu >= 0);
        }
    }

    // Which line, a or b, intersects l first? Return true if a intersects first
    friend auto isect_compare_unsafe(Rayd l, Rayd a, Rayd b) {
        // a hits at (an/ad)l.d, while b hits at (bn/bd)l.d
        auto an = cross(a.p - l.p, a.d), ad = cross(l.d, a.d);
        auto bn = cross(b.p - l.p, b.d), bd = cross(l.d, b.d);
        return an / ad < bn / bd;
    }

    friend auto isect_unsafe(Rayd u, Rayd v) {
        return u.p + cross(v.p - u.p, v.d) / cross(u.d, v.d) * u.d;
    }

    // Let w be the projection of u on this line, find k such that w=p+kd
    double coef(Pd2 u) const { return dot(u - p, d) / norm2(d); }
};
