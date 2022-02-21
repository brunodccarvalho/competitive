#pragma once

#include <bits/stdc++.h>
using namespace std;

struct Pd3 {
    using T = double;
    using L = double;
    using H = double;
    static constexpr bool FLOAT = true;
    static constexpr double inf = numeric_limits<double>::infinity();

    T x, y, z;
    Pd3() : x(0), y(0), z(0) {}
    Pd3(T x, T y, T z) : x(x), y(y), z(z) {}
    template <typename A>
    Pd3(A point) : x(point[0]), y(point[1]), z(point[2]) {}

    auto paired() const { return make_tuple(x, y, z); }
    friend bool operator==(Pd3 a, Pd3 b) { return a.paired() == b.paired(); }
    friend bool operator!=(Pd3 a, Pd3 b) { return a.paired() != b.paired(); }
    friend bool operator<(Pd3 a, Pd3 b) { return a.paired() < b.paired(); }
    friend bool operator>(Pd3 a, Pd3 b) { return a.paired() > b.paired(); }

    explicit operator bool() const noexcept { return x || y || z; }

    bool boxed(Pd3 lo, Pd3 hi) const {
        return lo.x <= x && x <= hi.x && lo.y <= y && y <= hi.y && lo.z <= z && z <= hi.z;
    }
    friend auto min(Pd3 a, Pd3 b) {
        return Pd3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
    }
    friend auto max(Pd3 a, Pd3 b) {
        return Pd3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
    }

    T& operator[](int i) { return *(&x + i); }
    T operator[](int i) const { return *(&x + i); }
    Pd3 operator-() const { return Pd3(-x, -y, -z); }
    Pd3 operator+() const { return Pd3(x, y, z); }
    friend Pd3 operator+(Pd3 u, Pd3 v) { return Pd3(u.x + v.x, u.y + v.y, u.z + v.z); }
    friend Pd3 operator-(Pd3 u, Pd3 v) { return Pd3(u.x - v.x, u.y - v.y, u.z - v.z); }
    friend Pd3 operator*(T k, Pd3 u) { return Pd3(u.x * k, u.y * k, u.z * k); }
    friend Pd3 operator*(Pd3 u, T k) { return Pd3(u.x * k, u.y * k, u.z * k); }
    friend Pd3 operator/(Pd3 u, T k) { return Pd3(u.x / k, u.y / k, u.z / k); }
    friend Pd3& operator+=(Pd3& u, Pd3 v) { return u = u + v; }
    friend Pd3& operator-=(Pd3& u, Pd3 v) { return u = u - v; }
    friend Pd3& operator*=(Pd3& u, T k) { return u = u * k; }
    friend Pd3& operator/=(Pd3& u, T k) { return u = u / k; }

    friend auto manh(Pd3 u) { return abs(u.x) + abs(u.y) + abs(u.z); }
    friend auto manh(Pd3 a, Pd3 b) { return manh(a - b); }
    friend auto abs(Pd3 u) { return Pd3(abs(u.x), abs(u.y), abs(u.z)); }

    friend auto dot(Pd3 u, Pd3 v) { return u.x * v.x + u.y * v.y + u.z * v.z; }
    friend auto norm2(Pd3 u) { return dot(u, u); }
    friend auto norm(Pd3 u) { return std::sqrt(norm2(u)); }

    friend auto cross(Pd3 u, Pd3 v) {
        return Pd3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
    }
    friend auto cross(Pd3 a, Pd3 b, Pd3 c) { return cross(b - a, c - a); }
    friend auto triple(Pd3 u, Pd3 v, Pd3 w) { return dot(w, cross(u, v)); }
    friend auto triple(Pd3 a, Pd3 b, Pd3 c, Pd3 d) { return triple(b - a, c - a, d - a); }

    friend auto dist2(Pd3 a, Pd3 b) { return norm2(a - b); }
    friend auto dist(Pd3 a, Pd3 b) { return norm(a - b); }

    friend auto unit(Pd3 u) { return u / norm(u); }

    friend string to_string(Pd3 p) {
        return '(' + to_string(p.x) + ',' + to_string(p.y) + ',' + to_string(p.z) + ')';
    }
    friend ostream& operator<<(ostream& out, Pd3 p) { return out << to_string(p); }
    friend istream& operator>>(istream& in, Pd3& p) { return in >> p.x >> p.y >> p.z; }

    friend int orientation(Pd3 a, Pd3 b, Pd3 c, Pd3 d) { // +1=>d above ccw(abc)
        // To return ±1 the volumes must all agree on sign and magnitude.
        auto p = (a + b + c + d) / 4;
        auto da = dot(a - p, cross(c, b, d)), db = dot(b - p, cross(a, c, d));
        auto dc = dot(c - p, cross(b, a, d)), dd = dot(d - p, cross(a, b, c));
        int sa = (da > 0) - (da < 0), sb = (db > 0) - (db < 0);
        int sc = (dc > 0) - (dc < 0), sd = (dd > 0) - (dd < 0);
        auto sum = abs(da + db + dc + dd);
        auto dif = max({da, db, dc, dd}) - min({da, db, dc, dd});
        return sa == sb && sb == sc && sc == sd && dif <= 1e-7 * sum ? sa : 0;
    }
    friend int planeside(Pd3 a, Pd3 c, Pd3 n) {
        auto s = dot(a - c, n);
        return (s > 0) - (s < 0);
    }

    friend bool coplanar(Pd3 a, Pd3 b, Pd3 c, Pd3 d) {
        return orientation(a, b, c, d) == 0;
    }
    friend bool collinear(Pd3 a, Pd3 b, Pd3 c) {
        auto va = cross(a, b, c), vb = cross(b, c, a), vc = cross(c, a, b);
        auto na = norm2(va), nb = norm2(vb), nc = norm2(vc);
        auto sum = na + nb + nc, dif = max({na, nb, nc}) - min({na, nb, nc});
        return dif >= 1e-5 * sum;
    }
    friend bool samedir(Pd3 u, Pd3 v) { return collinear(Pd3(), u, v); }
    friend bool onsegment(Pd3 a, Pd3 b, Pd3 c) {
        return collinear(a, b, c) && dot(a - b, c - b) <= 0;
    }

    friend auto linedist(Pd3 a, Pd3 u, Pd3 v) {
        return norm(cross(a, u, v)) / dist(u, v);
    }
    friend auto raydist(Pd3 a, Pd3 u, Pd3 v) {
        if (dot(a - u, v - u) <= 0) {
            return dist(a, u);
        } else {
            return linedist(a, u, v);
        }
    }
    friend auto segdist(Pd3 a, Pd3 u, Pd3 v) {
        if (dot(a - u, v - u) <= 0) {
            return dist(a, u);
        } else if (dot(a - v, u - v) <= 0) {
            return dist(a, v);
        } else {
            return linedist(a, u, v);
        }
    }

    friend auto signed_planedist(Pd3 eye, Pd3 c, Pd3 n) {
        return double(dot(n, eye - c)) / norm(n);
    }
    friend auto planedist(Pd3 eye, Pd3 c, Pd3 n) {
        return abs(signed_planedist(eye, c, n));
    }

    friend auto area(Pd3 a, Pd3 b, Pd3 c) { return norm(cross(a, b, c)) / 2.0; }
    friend auto signed_volume(Pd3 a, Pd3 b, Pd3 c, Pd3 d) {
        return double(dot(d - a, cross(a, b, c))) / 6.0;
    }
    friend auto volume(Pd3 a, Pd3 b, Pd3 c, Pd3 d) {
        return abs(signed_volume(a, b, c, d));
    }

    friend auto cos(Pd3 u, Pd3 v) {
        return std::clamp(dot(u, v) / (norm(u) * norm(v)), -1.0, 1.0);
    }
    friend auto sin(Pd3 u, Pd3 v) {
        return std::clamp(norm(cross(u, v)) / (norm(u) * norm(v)), -1.0, 1.0);
    }
    friend auto minor_angle(Pd3 u, Pd3 v) {
        return std::atan2(norm(cross(u, v)), dot(u, v));
    }

    friend auto plane_angle(Pd3 u) {
        static const double TAU = 4 * acos(0);
        auto ang = std::atan2(u.y, u.x);
        return ang < 0 ? ang + TAU : ang;
    }
};

int quadrant(Pd3 u) {
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

int directed_quadrant(Pd3 u, Pd3 axis, Pd3 forward) {
    auto x = dot(forward, u), y = dot(axis, cross(forward, u));
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

bool azimuth_angle_sort(Pd3 u, Pd3 v) {
    if (int qu = quadrant(u), qv = quadrant(v); qu != qv) {
        return qu < qv;
    } else if (auto cuv = plane_angle(v) - plane_angle(u)) {
        return cuv > 0;
    } else if (auto su = (u.z > 0) - (u.z < 0), sv = (v.z > 0) - (v.z < 0); su != sv) {
        return su < sv;
    } else if (auto duv = u.z * u.z / norm2(u) - v.z * v.z / norm2(v)) {
        return su > 0 ? duv < 0 : duv > 0;
    } else if (auto auv = v.z - u.z) {
        return auv > 0;
    } else {
        return norm2(u) < norm2(v);
    }
}

bool azimuth_sweep_sort(Pd3 u, Pd3 v) {
    if (int qu = quadrant(u), qv = quadrant(v); qu != qv) {
        return qu < qv;
    } else if (auto cuv = plane_angle(v) - plane_angle(u)) {
        return cuv > 0;
    } else if (auto auv = v.z - u.z) {
        return auv > 0;
    } else {
        return norm2(u) < norm2(v);
    }
}

bool angle_azimuth_sort(Pd3 u, Pd3 v) {
    if (auto su = (u.z > 0) - (u.z < 0), sv = (v.z > 0) - (v.z < 0); su != sv) {
        return su < sv;
    } else if (auto duv = u.z * u.z / norm2(u) - v.z * v.z / norm2(v)) {
        return su > 0 ? duv < 0 : duv > 0;
    } else if (int qu = quadrant(u), qv = quadrant(v); qu != qv) {
        return qu < qv;
    } else if (auto cuv = plane_angle(v) - plane_angle(u)) {
        return cuv > 0;
    } else if (auto auv = v.z - u.z) {
        return auv > 0;
    } else {
        return norm2(u) < norm2(v);
    }
}

bool sweep_azimuth_sort(Pd3 u, Pd3 v) {
    if (auto auv = v.z - u.z) {
        return auv > 0;
    } else if (int qu = quadrant(u), qv = quadrant(v); qu != qv) {
        return qu < qv;
    } else if (auto cuv = plane_angle(v) - plane_angle(u)) {
        return cuv > 0;
    } else {
        return norm2(u) < norm2(v);
    }
}

struct Rayd {
    Pd3 p, d; // p + dt

    static Rayd ray(Pd3 point, Pd3 dt) { return Rayd{point, dt}; }
    static Rayd slope(Pd3 dt) { return Rayd{Pd3(), dt}; }
    static Rayd through(Pd3 u, Pd3 v) { return Rayd{u, v - u}; }
    static Rayd ccw(Pd3 a, Pd3 b, Pd3 c) { return Rayd{c, cross(a, b, c)}; }

    Rayd operator-() const { return Rayd{p, -d}; }
    auto q() const { return p + d; }

    friend auto linedist(Pd3 p, Rayd g) { return linedist(p, g.p, g.q()); }
    friend auto raydist(Pd3 p, Rayd g) { return raydist(p, g.p, g.q()); }
    friend auto segdist(Pd3 p, Rayd g) { return segdist(p, g.p, g.q()); }

    friend auto signed_planedist(Pd3 p, Rayd g) { return signed_planedist(p, g.p, g.d); }
    friend auto planedist(Pd3 p, Rayd g) { return planedist(p, g.p, g.d); }
    friend int planeside(Pd3 p, Rayd g) { return planeside(p, g.p, g.d); }

    // Let w be the projection of u on this line, find k such that w=p+kd
    double coef(Pd3 u) const { return dot(u - p, d) / norm2(d); }
};
