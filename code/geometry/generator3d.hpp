#pragma once

#include "random.hpp"
#include "geometry/geometry3d.hpp"
#include "geometry/utils3d.hpp"

using Pointset = std::unordered_set<array<int64_t, 3>>;

auto floatint_point(double x, double y, double z) {
    if constexpr (Pt3::FLOAT) {
        return Pt3(x, y, z);
    } else {
        return Pt3(llround(x), llround(y), llround(z));
    }
}

auto floatint_insert_pointset(Pointset& pointset, Pt3 point) {
    static constexpr double MULT = 1e5;
    static constexpr array<array<int, 3>, 8> integer_box = {{
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {0, 1, 1},
        {1, 0, 0},
        {1, 0, 1},
        {1, 1, 0},
        {1, 1, 1},
    }};
    if constexpr (Pt3::FLOAT) {
        int64_t x = floor(point.x * MULT);
        int64_t y = floor(point.y * MULT);
        int64_t z = floor(point.z * MULT);
        bool ok = true;
        for (auto [dx, dy, dz] : integer_box) {
            ok &= !pointset.count({x + dx, y + dy, z + dz});
        }
        if (ok) {
            for (auto [dx, dy, dz] : integer_box) {
                pointset.insert({x + dx, y + dy, z + dz});
            }
        }
        return ok;
    } else {
        int64_t x = point.x, y = point.y, z = point.z;
        return pointset.insert({x, y, z}).second;
    }
}

enum class PointDistrib {
    CUBE,                  // interior of axis-aligned cube side 2R
    CUBE_SURFACE,          // faces and edges of axis-aligned cube side 2R
    CUBE_EDGES,            // edges of axis-aligned cube (~16R points)
    CUBE_CORNERS,          // sphere quadrant on corners of axis-aligned cube side 2R
    CUBE_ROUNDED,          // edges of axis-aligned cube, faces are circles (~16R points)
    CYLINDER,              // axis-aligned cylinder height 2R and radius R
    CYLINDER_SURFACE,      // surface of a cylinder of height 2R and radius R
    BALL,                  // interior of a sphere of radius R (rounded for ints)
    SPHERE,                // surface of a sphere of radius R (rounded for ints)
    ELLIPTIC_PARABOLOID,   // (x,y,(x²+y²)/R), -R<=(x,y)<=R
    HYPERPOLIC_PARABOLOID, // (x,y,(x²-y²)/R), -R<=(x,y)<=R
    HYPERBOLOID,           // (x,y,√(x²+y²)+c), -R<=(x,y)<=R
    STARLINES,             // (ax,bx,cx) for {a,b,c} in {-1,0,1}, -R<=x<=R (~26R points)
    END,
};

auto rand_point_distribution() {
    static discrete_distribution<int> dist({2, 3, 4, 4, 4, 2, 4, 2, 4, 3, 2, 2, 2});
    return PointDistrib(dist(mt));
}

string to_string(PointDistrib distr) {
    static const string names[] = {
        "cube"s,
        "cube-surface"s,
        "cube-edges"s,
        "cube-corners"s,
        "cube-rounded"s,
        "cylinder"s,
        "cylinder-surface"s,
        "ball"s,
        "sphere"s,
        "elliptic-paraboloid"s,
        "hyperbolic-paraboloid"s,
        "hyperboloid"s,
        "starlines"s,
        "END"s,
    };
    return names[int(distr)];
}

void rotate_coordinates_left(vector<Pt3>& pts) {
    for (auto& p : pts) {
        p = Pt3(p.y, p.z, p.x);
    }
}

void rotate_coordinates_right(vector<Pt3>& pts) {
    for (auto& p : pts) {
        p = Pt3(p.z, p.x, p.y);
    }
}

auto rand_cube(Pt3::T R = 2000) {
    auto x = rand_unif<Pt3::T>(-R, R);
    auto y = rand_unif<Pt3::T>(-R, R);
    auto z = rand_unif<Pt3::T>(-R, R);
    return Pt3(x, y, z);
}

auto rand_cube_surface(Pt3::T R = 2000) {
    static boold coind(0.5);
    auto x = rand_unif<Pt3::T>(-R, R);
    auto y = rand_unif<Pt3::T>(-R, R);
    auto z = rand_unif<Pt3::T>(-R, R);
    int i = rand_unif<int>(0, 2);
    Pt3 point(x, y, z);
    point[i] = coind(mt) ? R : -R;
    return point;
}

auto rand_cube_edges(Pt3::T R = 2000) {
    static boold coind(0.5);
    auto x = rand_unif<Pt3::T>(-R, R);
    auto y = rand_unif<Pt3::T>(-R, R);
    auto z = rand_unif<Pt3::T>(-R, R);
    auto [i, j] = diff_unif<int>(0, 2);
    Pt3 point(x, y, z);
    point[i] = coind(mt) ? R : -R;
    point[j] = coind(mt) ? R : -R;
    return point;
}

auto rand_cube_corners(Pt3::T R = 2000, Pt3::T r = 0) {
    static boold coind(0.5);
    static normald dist(0, 1);
    r = r == 0 ? R / 3 : r, R -= r;
    auto x = dist(mt), y = dist(mt), z = dist(mt);
    auto n = sqrt(x * x + y * y + z * z);
    Pt3 point = abs(floatint_point(r * x / n, r * y / n, r * z / n));
    point[0] = coind(mt) ? R + point[0] : -R - point[0];
    point[1] = coind(mt) ? R + point[1] : -R - point[1];
    point[2] = coind(mt) ? R + point[2] : -R - point[2];
    return point;
}

auto rand_cube_rounded(Pt3::T R = 2000) {
    static boold coind(0.5);
    static normald dist(0, 1);
    int i = rand_unif<int>(0, 2);
    auto side = coind(mt) ? R : -R;
    auto x = dist(mt), y = dist(mt), z = dist(mt);
    auto n = sqrt((i != 0) * x * x + (i != 1) * y * y + (i != 2) * z * z);
    Pt3 point = floatint_point(R * x / n, R * y / n, R * z / n);
    point[i] = side;
    return point;
}

auto rand_cylinder(Pt3::T R = 2000, int repulsion = 1) {
    static normald dist(0, 1);
    auto x = dist(mt), y = dist(mt);
    auto z = rand_unif<Pt3::T>(-R, R);
    auto r = rand_wide<double>(0, R, repulsion);
    auto n = sqrt(x * x + y * y);
    return floatint_point(r * x / n, r * y / n, z);
}

auto rand_cylinder_surface(Pt3::T R = 2000) {
    static normald dist(0, 1);
    auto x = dist(mt), y = dist(mt);
    auto z = rand_unif<Pt3::T>(-R, R);
    auto n = sqrt(x * x + y * y);
    return floatint_point(R * x / n, R * y / n, z);
}

auto rand_ball(Pt3::T R = 2000, int repulsion = 1) {
    static normald dist(0, 1);
    auto x = dist(mt), y = dist(mt), z = dist(mt);
    auto n = sqrt(x * x + y * y + z * z);
    auto r = rand_wide<double>(0, R, repulsion);
    return floatint_point(r * x / n, r * y / n, r * z / n);
}

auto rand_sphere(Pt3::T R = 2000) {
    static normald dist(0, 1);
    auto x = dist(mt), y = dist(mt), z = dist(mt);
    auto n = sqrt(x * x + y * y + z * z);
    return floatint_point(R * x / n, R * y / n, R * z / n);
}

auto rand_elliptic_paraboloid(Pt3::T R = 2000, Pt3::T c = 0) {
    auto x = rand_unif<Pt3::T>(-R, R);
    auto y = rand_unif<Pt3::T>(-R, R);
    return Pt3(x, y, (x * x + y * y) / R + c);
}

auto rand_hyperbolic_paraboloid(Pt3::T R = 2000, Pt3::T c = 0) {
    auto x = rand_unif<Pt3::T>(-R, R);
    auto y = rand_unif<Pt3::T>(-R, R);
    return Pt3(x, y, (x * x - y * y) / R + c);
}

auto rand_hyperboloid(Pt3::T R = 2000, Pt3::T c = 10) {
    static boold coind(0.5);
    auto r = R / 2.0;
    auto x = rand_unif<Pt3::T>(-r, r);
    auto y = rand_unif<Pt3::T>(-r, r);
    auto z = sqrt(x * x + y * y) + c;
    return floatint_point(x, y, coind(mt) ? z : -z);
}

auto rand_starlines(Pt3::T R = 2000, int gravity = -1) {
    static discrete_distribution sided({3, 2, 3});
    int a, b, c;
    do {
        a = sided(mt) - 1, b = sided(mt) - 1, c = sided(mt) - 1;
    } while (a == 0 && b == 0 && c == 0);
    auto x = rand_grav<Pt3::T>(-R, R, gravity);
    return Pt3(a * x, b * x, c * x);
}

auto add_collinear_between(int N, Pointset& pointset, vector<Pt3>& pts) {
    int S = pts.size();
    int runs = 100 * N;
    while (N && runs-- && S > 1) {
        auto [i, j] = diff_unif<int>(0, S - 1);
        auto uv = pts[j] - pts[i];
        Pt3 point;

        if constexpr (Pt3::FLOAT) {
            constexpr double EPS = 1e-5;
            auto a = rand_unif<int>(0 + EPS, 1 - EPS);
            point = pts[i] + a * uv;
        } else {
            auto g = abs(gcd(gcd(uv.x, uv.y), uv.z));
            auto m = rand_unif<Pt3::T>(0, g);
            point = pts[i] + uv * m / g;
        }

        if (floatint_insert_pointset(pointset, point)) {
            N--, pts.push_back(point), S++;
        }
    }
}

auto add_coplanar_between(int N, Pointset& pointset, vector<Pt3>& pts) {
    int S = pts.size();
    int runs = 100 * N;
    while (N && runs-- && S > 2) {
        auto sample = int_sample<int>(3, 0, S);
        int i = sample[0], j = sample[1], k = sample[2];
        auto uv = pts[j] - pts[i];
        auto uw = pts[k] - pts[i];
        Pt3 point;

        if constexpr (Pt3::FLOAT) {
            constexpr double EPS = 1e-5;
            auto a = rand_unif<double>(0 + EPS, 1 - EPS);
            auto b = rand_unif<double>(0 + EPS, 1 - EPS);
            if (a + b > 1.0) {
                a = 1.0 - a;
                b = 1.0 - b;
            }
            point = pts[i] + a * uv + b * uw;
        } else {
            auto guv = abs(gcd(gcd(uv.x, uv.y), uv.z));
            auto guw = abs(gcd(gcd(uw.x, uw.y), uw.z));
            auto muv = rand_unif<Pt3::T>(0, guv);
            auto muw = rand_unif<Pt3::T>(0, guw);
            if (1.0 * muv / guv + 1.0 * muw / guw > 1.0) {
                muv = guv - muv,
                muw = guw - muw; // reflection, point lied outside triangle
            }
            point = pts[i] + muv * uv / guv + muw * uw / guw;
        }

        if (floatint_insert_pointset(pointset, point)) {
            N--, pts.push_back(point), S++;
        }
    }
}

auto rand_rotrefl(vector<Pt3>& pts) {
    static boold coind(0.5);
    for (int i = 0; i <= 2; i++) {
        if (coind(mt)) {
            for (auto& pt : pts) {
                pt[i] = -pt[i];
            }
        }
    }
    static constexpr array<array<int, 2>, 3> dds = {{{0, 1}, {1, 2}, {2, 0}}};
    for (auto [i, j] : dds) {
        if (coind(mt)) {
            for (auto& pt : pts) {
                swap(pt[i], pt[j]);
            }
        }
    }
}

auto generate_point(PointDistrib dist, Pt3::T R = 2000) {
    switch (dist) {
    case PointDistrib::CUBE:
        return rand_cube(R);
    case PointDistrib::CUBE_SURFACE:
        return rand_cube_surface(R);
    case PointDistrib::CUBE_EDGES:
        return rand_cube_edges(R);
    case PointDistrib::CUBE_CORNERS:
        return rand_cube_corners(R);
    case PointDistrib::CUBE_ROUNDED:
        return rand_cube_rounded(R);
    case PointDistrib::CYLINDER:
        return rand_cylinder(R);
    case PointDistrib::CYLINDER_SURFACE:
        return rand_cylinder_surface(R);
    case PointDistrib::BALL:
        return rand_ball(R);
    case PointDistrib::SPHERE:
        return rand_sphere(R);
    case PointDistrib::ELLIPTIC_PARABOLOID:
        return rand_elliptic_paraboloid(R);
    case PointDistrib::HYPERPOLIC_PARABOLOID:
        return rand_hyperbolic_paraboloid(R);
    case PointDistrib::HYPERBOLOID:
        return rand_hyperboloid(R);
    case PointDistrib::STARLINES:
        return rand_starlines(R);
    default:
        throw runtime_error("Invalid distribution");
    }
}

auto generate_points(int N, PointDistrib dist, int L = 0, int C = 0, Pt3::T R = 2000) {
    Pointset pointset;
    vector<Pt3> pts;
    while (N) {
        auto point = generate_point(dist, R);
        if (floatint_insert_pointset(pointset, point)) {
            pts.push_back(point), N--;
        }
    }
    add_coplanar_between(C, pointset, pts);
    add_collinear_between(L, pointset, pts);
    rand_rotrefl(pts);
    assert(bounding_box_size(pts).boxed(Pt3(), 5 * Pt3(R, R, R)));
    return pts;
}

auto hypercube32(Pt3::T R = 10, Pt3::T h = 3, Pt3::T r = 4) {
    vector<Pt3> pts;
    for (Pt3::T x : {-R, R}) {
        for (Pt3::T y : {-R, R}) {
            for (Pt3::T z : {-R, R}) {
                pts.push_back(Pt3(x, y, z));
            }
        }
    }
    for (int dim = 0; dim <= 2; dim++) {
        for (int side : {-1, 1}) {
            Pt3 p(0, 0, 0);
            p[dim] = side * (R + h);
            for (int a : {-1, 1}) {
                for (int b : {-1, 1}) {
                    p[(dim + 1) % 3] = a * r;
                    p[(dim + 2) % 3] = b * r;
                    pts.push_back(p);
                }
            }
        }
    }
    return pts;
}
