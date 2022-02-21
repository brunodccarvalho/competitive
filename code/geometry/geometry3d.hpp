#pragma once

#include <bits/stdc++.h>
using namespace std;

__int128_t abs(__int128_t x) { return x >= 0 ? x : -x; }

// For 3D computational geometry problems.

struct Pt3 {
    using T = int64_t;    // points, vectors, crosses, coefs, manh -- integer/frac/quot
    using L = __int128_t; // dots, norm2, dist2 -- integer/frac/quot
    using H = double;     // huge (sphere predicates) -- int128/double
    static constexpr bool FLOAT = false;

    T x, y, z;
    Pt3() : x(0), y(0), z(0) {}
    Pt3(T x, T y, T z) : x(x), y(y), z(z) {}

    auto paired() const { return make_tuple(x, y, z); }
    friend bool operator==(Pt3 a, Pt3 b) { return a.paired() == b.paired(); }
    friend bool operator!=(Pt3 a, Pt3 b) { return a.paired() != b.paired(); }
    friend bool operator<(Pt3 a, Pt3 b) { return a.paired() < b.paired(); }
    friend bool operator>(Pt3 a, Pt3 b) { return a.paired() > b.paired(); }
    friend bool operator<=(Pt3 a, Pt3 b) { return a.paired() <= b.paired(); }
    friend bool operator>=(Pt3 a, Pt3 b) { return a.paired() >= b.paired(); }

    explicit operator bool() const noexcept { return x || y || z; }

    bool boxed(Pt3 lo, Pt3 hi) const {
        return lo.x <= x && x <= hi.x && lo.y <= y && y <= hi.y && lo.z <= z && z <= hi.z;
    }
    friend auto min(Pt3 a, Pt3 b) {
        return Pt3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
    }
    friend auto max(Pt3 a, Pt3 b) {
        return Pt3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
    }

    T& operator[](int i) { return *(&x + i); }
    T operator[](int i) const { return *(&x + i); }
    Pt3 operator-() const { return Pt3(-x, -y, -z); }
    Pt3 operator+() const { return Pt3(x, y, z); }
    friend Pt3 operator+(Pt3 u, Pt3 v) { return Pt3(u.x + v.x, u.y + v.y, u.z + v.z); }
    friend Pt3 operator-(Pt3 u, Pt3 v) { return Pt3(u.x - v.x, u.y - v.y, u.z - v.z); }
    friend Pt3 operator*(T k, Pt3 u) { return Pt3(u.x * k, u.y * k, u.z * k); }
    friend Pt3 operator*(Pt3 u, T k) { return Pt3(u.x * k, u.y * k, u.z * k); }
    friend Pt3 operator/(Pt3 u, T k) { return Pt3(u.x / k, u.y / k, u.z / k); }
    friend Pt3& operator+=(Pt3& u, Pt3 v) { return u = u + v; }
    friend Pt3& operator-=(Pt3& u, Pt3 v) { return u = u - v; }
    friend Pt3& operator*=(Pt3& u, T k) { return u = u * k; }
    friend Pt3& operator/=(Pt3& u, T k) { return u = u / k; }

    friend auto manh(Pt3 u) { return abs(u.x) + abs(u.y) + abs(u.z); }
    friend auto manh(Pt3 a, Pt3 b) { return manh(a - b); }
    friend auto abs(Pt3 u) { return Pt3(abs(u.x), abs(u.y), abs(u.z)); }

    friend auto dot(Pt3 u, Pt3 v) { return L(u.x) * v.x + L(u.y) * v.y + L(u.z) * v.z; }
    friend auto norm2(Pt3 u) { return dot(u, u); }
    friend auto norm(Pt3 u) { return std::sqrt(double(norm2(u))); }

    friend auto cross(Pt3 u, Pt3 v) {
        return Pt3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
    }
    friend auto cross(Pt3 a, Pt3 b, Pt3 c) { return cross(b - a, c - a); }
    friend auto triple(Pt3 u, Pt3 v, Pt3 w) { return dot(w, cross(u, v)); }
    friend auto triple(Pt3 a, Pt3 b, Pt3 c, Pt3 d) { return triple(b - a, c - a, d - a); }

    friend auto dist2(Pt3 a, Pt3 b) { return norm2(a - b); }
    friend auto dist(Pt3 a, Pt3 b) { return norm(a - b); }

    friend auto int_norm(Pt3 u) { return abs(gcd(u.x, gcd(u.y, u.z))); }
    friend auto int_unit(Pt3 u) { return u / int_norm(u); }

    friend string to_string(Pt3 p) {
        return '(' + to_string(p.x) + ',' + to_string(p.y) + ',' + to_string(p.z) + ')';
    }
    friend ostream& operator<<(ostream& out, Pt3 p) { return out << to_string(p); }
    friend istream& operator>>(istream& in, Pt3& p) { return in >> p.x >> p.y >> p.z; }

    friend int orientation(Pt3 a, Pt3 b, Pt3 c, Pt3 d) { // +1=>d above ccw(abc)
        auto s = dot(d - a, cross(a, b, c));
        return (s > 0) - (s < 0);
    }
    friend int planeside(Pt3 a, Pt3 c, Pt3 n) {
        auto s = dot(a - c, n);
        return (s > 0) - (s < 0);
    }

    friend bool coplanar(Pt3 a, Pt3 b, Pt3 c, Pt3 d) {
        return orientation(a, b, c, d) == 0;
    }
    friend bool collinear(Pt3 a, Pt3 b, Pt3 c) { return cross(a, b, c) == Pt3(); }
    friend bool samedir(Pt3 u, Pt3 v) { return cross(u, v) == Pt3(); }
    friend bool onsegment(Pt3 a, Pt3 b, Pt3 c) {
        return collinear(a, b, c) && dot(a - b, c - b) <= 0;
    }

    friend auto linedist(Pt3 a, Pt3 u, Pt3 v) {
        return norm(cross(a, u, v)) / dist(u, v);
    }
    friend auto raydist(Pt3 a, Pt3 u, Pt3 v) {
        if (dot(a - u, v - u) <= 0) {
            return dist(a, u);
        } else {
            return linedist(a, u, v);
        }
    }
    friend auto segdist(Pt3 a, Pt3 u, Pt3 v) {
        if (dot(a - u, v - u) <= 0) {
            return dist(a, u);
        } else if (dot(a - v, u - v) <= 0) {
            return dist(a, v);
        } else {
            return linedist(a, u, v);
        }
    }

    friend auto signed_planedist(Pt3 eye, Pt3 c, Pt3 n) {
        return double(dot(n, eye - c)) / norm(n);
    }
    friend auto planedist(Pt3 eye, Pt3 c, Pt3 n) {
        return abs(signed_planedist(eye, c, n));
    }

    friend auto area(Pt3 a, Pt3 b, Pt3 c) { return norm(cross(a, b, c)) / 2.0; }
    friend auto signed_volume(Pt3 a, Pt3 b, Pt3 c, Pt3 d) {
        return double(dot(d - a, cross(a, b, c))) / 6.0;
    }
    friend auto volume(Pt3 a, Pt3 b, Pt3 c, Pt3 d) {
        return abs(signed_volume(a, b, c, d));
    }

    friend auto cos(Pt3 u, Pt3 v) {
        return std::clamp(double(dot(u, v)) / (norm(u) * norm(v)), -1.0, 1.0);
    }
    friend auto sin(Pt3 u, Pt3 v) {
        return std::clamp(norm(cross(u, v)) / (norm(u) * norm(v)), -1.0, 1.0);
    }
    friend auto minor_angle(Pt3 u, Pt3 v) {
        return std::atan2(norm(cross(u, v)), dot(u, v));
    }
};

int quadrant(Pt3 u) {
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

int directed_quadrant(Pt3 u, Pt3 axis, Pt3 forward) {
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

// Sort by azimuth (2d projection), then by angle from -z to +z
bool azimuth_angle_sort(Pt3 u, Pt3 v) {
    if (int qu = quadrant(u), qv = quadrant(v); qu != qv) {
        return qu < qv;
    } else if (auto cuv = cross(u, v).z) {
        return cuv > 0;
    } else if (auto su = (u.z > 0) - (u.z < 0), sv = (v.z > 0) - (v.z < 0); su != sv) {
        return su < sv;
    } else if (auto duv = u.z * u.z * norm2(v) - v.z * v.z * norm2(u)) {
        return su > 0 ? duv < 0 : duv > 0;
    } else if (auto auv = v.z - u.z) {
        return auv > 0;
    } else {
        return norm2(u) < norm2(v);
    }
}

// Sort by azimuth (2d projection), then from -z to +z
bool azimuth_sweep_sort(Pt3 u, Pt3 v) {
    if (int qu = quadrant(u), qv = quadrant(v); qu != qv) {
        return qu < qv;
    } else if (auto cuv = cross(u, v).z) {
        return cuv > 0;
    } else if (auto auv = v.z - u.z) {
        return auv > 0;
    } else {
        return norm2(u) < norm2(v);
    }
}

// Sort by angle from -z to +z, then by azimuth (2d projection)
bool angle_azimuth_sort(Pt3 u, Pt3 v) {
    if (auto su = (u.z > 0) - (u.z < 0), sv = (v.z > 0) - (v.z < 0); su != sv) {
        return su < sv;
    } else if (auto duv = u.z * u.z * norm2(v) - v.z * v.z * norm2(u)) {
        return su > 0 ? duv < 0 : duv > 0;
    } else if (int qu = quadrant(u), qv = quadrant(v); qu != qv) {
        return qu < qv;
    } else if (auto cuv = cross(u, v).z) {
        return cuv > 0;
    } else if (auto auv = v.z - u.z) {
        return auv > 0;
    } else {
        return norm2(u) < norm2(v);
    }
}

// Sort from -z to +z, then by azimuth (2d projection)
bool sweep_azimuth_sort(Pt3 u, Pt3 v) {
    if (auto auv = v.z - u.z) {
        return auv > 0;
    } else if (int qu = quadrant(u), qv = quadrant(v); qu != qv) {
        return qu < qv;
    } else if (auto cuv = cross(u, v).z) {
        return cuv > 0;
    } else {
        return norm2(u) < norm2(v);
    }
}

// Sort by azimuth (2d projection), then by angle from -z to +z
auto azimuth_angle_sorter(Pt3 pivot, Pt3 axis, Pt3 forward) {
    assert(axis && forward);
    return [=](Pt3 u, Pt3 v) -> bool {
        u -= pivot, v -= pivot;
        int qu = directed_quadrant(u, axis, forward);
        int qv = directed_quadrant(v, axis, forward);
        if (qu != qv) {
            return qu < qv;
        } else if (auto cuv = dot(axis, cross(u, v))) {
            return cuv > 0;
        } else if (auto duv = dot(cross(u, axis), cross(u, v))) {
            return duv > 0;
        } else if (auto auv = dot(v - u, axis)) {
            return auv > 0;
        } else {
            return norm2(u) < norm2(v);
        }
    };
}

// Sort by azimuth (2d projection), then from -z to +z
auto azimuth_sweep_sorter(Pt3 pivot, Pt3 axis, Pt3 forward) {
    assert(axis && forward);
    return [=](Pt3 u, Pt3 v) -> bool {
        u -= pivot, v -= pivot;
        int qu = directed_quadrant(u, axis, forward);
        int qv = directed_quadrant(v, axis, forward);
        if (qu != qv) {
            return qu < qv;
        } else if (auto cuv = dot(axis, cross(u, v))) {
            return cuv > 0;
        } else if (auto auv = dot(v - u, axis)) {
            return auv > 0;
        } else {
            return norm2(u) < norm2(v);
        }
    };
}

// Sort by angle from -z to +z, then by azimuth (2d projection)
// ! Requires huge norms (dot(u,axis)^2 norm^2(v) must fit in L)
auto angle_azimuth_sorter(Pt3 pivot, Pt3 axis, Pt3 forward) {
    assert(axis && forward);
    return [=](Pt3 u, Pt3 v) {
        u -= pivot, v -= pivot;
        auto du = dot(u, axis), dv = dot(v, axis);
        int qu = directed_quadrant(u, axis, forward);
        int qv = directed_quadrant(v, axis, forward);
        if (auto su = (du > 0) - (du < 0), sv = (dv > 0) - (dv < 0); su != sv) {
            return su < sv;
        } else if (auto duv = du * du * norm2(v) - dv * dv * norm2(u)) {
            return su > 0 ? duv < 0 : duv > 0; // ! ^ The above requires huge norms
        } else if (qu != qv) {
            return qu < qv;
        } else if (auto cuv = dot(axis, cross(u, v))) {
            return cuv > 0;
        } else if (auto auv = dot(v - u, axis)) {
            return auv > 0;
        } else {
            return norm2(u) < norm2(v);
        }
    };
}

// Sort from -z to +z, then by azimuth (2d projection)
auto sweep_azimuth_sorter(Pt3 pivot, Pt3 axis, Pt3 forward) {
    assert(axis && forward);
    return [=](Pt3 u, Pt3 v) {
        u -= pivot, v -= pivot;
        auto du = dot(u, axis), dv = dot(v, axis);
        int qu = directed_quadrant(u, axis, forward);
        int qv = directed_quadrant(v, axis, forward);
        if (du != dv) {
            return du < dv;
        } else if (qu != qv) {
            return qu < qv;
        } else if (auto cuv = dot(axis, cross(u, v))) {
            return cuv > 0;
        } else {
            return norm2(u) < norm2(v);
        }
    };
}

struct Ray {
    Pt3 p, d; // p + dt

    static Ray ray(Pt3 point, Pt3 dt) { return Ray{point, dt}; }
    static Ray slope(Pt3 dt) { return Ray{Pt3(), dt}; }
    static Ray through(Pt3 u, Pt3 v) { return Ray{u, v - u}; }
    static Ray ccw(Pt3 a, Pt3 b, Pt3 c) { return Ray{c, cross(a, b, c)}; }

    Ray operator-() const { return Ray{p, -d}; }
    auto q() const { return p + d; }

    friend auto linedist(Pt3 p, Ray l) { return linedist(p, l.p, l.q()); }
    friend auto raydist(Pt3 p, Ray l) { return raydist(p, l.p, l.q()); }
    friend auto segdist(Pt3 p, Ray l) { return segdist(p, l.p, l.q()); }

    friend auto signed_planedist(Pt3 p, Ray l) { return signed_planedist(p, l.p, l.d); }
    friend auto planedist(Pt3 p, Ray l) { return planedist(p, l.p, l.d); }
    friend int planeside(Pt3 p, Ray l) { return planeside(p, l.p, l.d); }

    // Let w be the projection of u on this line, find k such that w=p+kd
    double coef(Pt3 u) const { return double(dot(u - p, d)) / norm2(d); }
};
