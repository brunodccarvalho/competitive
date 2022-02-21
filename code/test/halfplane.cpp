#include "test_utils.hpp"
#include "geometry/halfplane.hpp"

void run_halfplane_unit_test(const vector<Ray>& hp, const string& header) {
    println("====== {} ({} halfplanes)", header, hp.size());

    auto hull = halfplane_isect(hp);
    println("Lines:");
    for (int i = 0, N = hp.size(); i < N; i++) {
        println("[{}]: {} + t{}", i, hp[i].p, hp[i].d);
    }

    println("Hull: {}", hull);
}

void unit_test_halfplane_isect() {
    vector<Ray> hp;

    hp = {Ray::ray(Pt2(0, -10), Pt2(1, 0)),  Ray::ray(Pt2(0, -15), Pt2(1, 1)),
          Ray::ray(Pt2(10, 0), Pt2(0, 1)),   Ray::ray(Pt2(15, 0), Pt2(-1, 1)),
          Ray::ray(Pt2(0, 10), Pt2(-1, 0)),  Ray::ray(Pt2(0, 15), Pt2(-1, -1)),
          Ray::ray(Pt2(-10, 0), Pt2(0, -1)), Ray::ray(Pt2(-15, 0), Pt2(1, -1))};
    run_halfplane_unit_test(hp, "regular-octagon");

    hp = {Ray::ray(Pt2(0, -10), Pt2(1, 0)),  Ray::ray(Pt2(0, -15), Pt2(1, 1)),
          Ray::ray(Pt2(10, 0), Pt2(0, 1)),   Ray::ray(Pt2(15, 0), Pt2(-1, 1)),
          Ray::ray(Pt2(0, 10), Pt2(-1, 0)),  Ray::ray(Pt2(0, 15), Pt2(-1, -1)),
          Ray::ray(Pt2(-10, 0), Pt2(0, -1)), Ray::ray(Pt2(-15, 0), Pt2(1, -1)),
          Ray::ray(Pt2(0, -20), Pt2(1, 1)),  Ray::ray(Pt2(20, 0), Pt2(-2, 1))};
    run_halfplane_unit_test(hp, "octagon-cuts-3");

    hp = {Ray::ray(Pt2(0, -10), Pt2(1, 0)),  Ray::ray(Pt2(0, -15), Pt2(1, 1)),
          Ray::ray(Pt2(10, 0), Pt2(0, 1)),   Ray::ray(Pt2(15, 0), Pt2(-1, 1)),
          Ray::ray(Pt2(0, 10), Pt2(-1, 0)),  Ray::ray(Pt2(0, 15), Pt2(-1, -1)),
          Ray::ray(Pt2(-10, 0), Pt2(0, -1)), Ray::ray(Pt2(-15, 0), Pt2(1, -1)),
          Ray::ray(Pt2(0, -20), Pt2(1, 1)),  Ray::ray(Pt2(21, 0), Pt2(-2, 1))};
    run_halfplane_unit_test(hp, "octagon-wont-cut-3");

    hp = {Ray::ray(Pt2(0, -10), Pt2(10, 1)), Ray::ray(Pt2(0, 10), Pt2(-10, 1)),
          Ray::ray(Pt2(10, 0), Pt2(1, -10)), Ray::ray(Pt2(-10, 0), Pt2(1, 10))};
    run_halfplane_unit_test(hp, "half-out-cube");

    hp = {Ray::ray(Pt2(0, 0), Pt2(1, 0)),    Ray::ray(Pt2(10, 0), Pt2(3, 1)),
          Ray::ray(Pt2(-10, 0), Pt2(1, -1)), Ray::ray(Pt2(-20, 0), Pt2(1, -10)),
          Ray::ray(Pt2(-19, 0), Pt2(1, -3)), Ray::ray(Pt2(12, 0), Pt2(0, 1))};
    run_halfplane_unit_test(hp, "parabola");
}

int main() {
    RUN_BLOCK(unit_test_halfplane_isect());
    return 0;
}
