#include "test_utils.hpp"
#include "numeric/quadratic.hpp"
#include "numeric/frac.hpp"

void unit_test_quadratic() {
    using quad = quadratic<frac<int>>;
    quad f1(1, -2, 1);
    quad f2(1, 3, 2);
    quad f3(3, -7, 1);
    quad f4(1, 1, 1);
    quad f5(3, 8, -7);

    auto zeros = [&](auto f) {
        println("zeros({},{},{}) = {}\n", f.a, f.b, f.c, f.roots());
    };
    auto eval = [&](auto f, auto x) {
        println("f({})={}x^2+{}x+{}={}\n", x, f.a, f.b, f.c, f(x));
    };

    zeros(f1);
    zeros(f2);
    zeros(f3);
    zeros(f4);
    zeros(f5);
    eval(f1, 1);
    eval(f1, f1.argvertex());
    eval(f1, f1.argmin(2, 4));
    eval(f1, f1.argmax(2, 4));
}

int main() {
    RUN_BLOCK(unit_test_quadratic());
    return 0;
}
