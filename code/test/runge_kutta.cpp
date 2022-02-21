#include "test_utils.hpp"
#include "numeric/runge_kutta.hpp"

double sine(double x) { return sin(x); }
double cosine(double x) { return cos(x); }

void unit_test_simpson() {
    for (int n = 1; n <= 20; n++) {
        println("simpson(N={}): {:2.15f}", 1 << n, simpson(1 << n, 0.0, 1.0, sine));
    }
}

void unit_test_runge_kutta_explicit() {
    auto f = [](double, double, double z) { return z; };
    auto g = [](double, double y, double z) { return (1 - y * y) * z - y; };

    int N = 10000, S = 50;
    auto trace1 = explicit_rk4_two(N, 0.0, 1.0, 0.0, 25.0, f, g);
    println("--- Explicit van der pol");
    for (int i = 0; i < S; i++) {
        auto [x, y, z] = trace1[N / S * i];
        println("t:{:7.3f} y:{:7.3f} z:{:7.3f}", x, y, z);
    }
}

void unit_test_runge_kutta_implicit() {
    auto f = [](double x, double y, double z) { return 2 * cos(z) - sin(y) - x; };

    int N = 10000, S = 50;
    auto trace1 = implicit_rk1_one(N, 20, 0.0, 1.0, 1.0, 0.5, 1e-9, f);
    println("--- Implicit trig");
    for (int i = 0; i < S; i++) {
        auto [x, y, z] = trace1[N / S * i];
        println("t:{:7.3f} y:{:7.3f} z:{:7.3f}  f: {}", x, y, z, f(x, y, z));
    }
}

int main() {
    RUN_BLOCK(unit_test_simpson());
    RUN_BLOCK(unit_test_runge_kutta_explicit());
    RUN_BLOCK(unit_test_runge_kutta_implicit());
    return 0;
}
