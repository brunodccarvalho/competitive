#include "test_utils.hpp"
#include "linear/simplex.hpp"

void run_test(simplex<double>& lp) {
    auto normal_primal = lp;
    auto normal_dual = lp;
    auto dual_primal = lp;
    auto dual_dual = lp;
    dual_primal.dualize();
    dual_dual.dualize();

    println("== N={} M={} phi={}", lp.N, lp.M, lp.potential());

    auto ok1 = normal_primal.run_primal_dual();
    println("Normal run_primal_dual()");
    println("  result: {}", to_string(ok1));
    println("     ans: {}", normal_primal.extract());
    println("     opt: {}", normal_primal.optimum);

    auto ok2 = normal_dual.run_dual_primal();
    println("Normal run_dual_primal()");
    println(" result: {}", to_string(ok2));
    println("    ans: {}", normal_dual.extract());
    println("    opt: {}", normal_dual.optimum);

    auto ok3 = dual_primal.run_primal_dual();
    println("Dual run_primal_dual()");
    println(" result: {}", to_string(ok3));
    println("    ans: {}", dual_primal.extract());
    println("    opt: {}", dual_primal.optimum);

    auto ok4 = dual_dual.run_dual_primal();
    println("Dual run_dual_primal()");
    println(" result: {}", to_string(ok4));
    println("    ans: {}", dual_dual.extract());
    println("    opt: {}", dual_dual.optimum);
}

void unit_test_simplex() {
    simplex<double> lp(3, 3);

    lp.A = {
        {3, 2, 6},
        {2, -1, 7},
        {4, 5, 1},
    };
    lp.B = {15, 4, 13};
    lp.C = {2, 4, 3};

    run_test(lp); // 12.416667, (0, 2.4166667, 0.9166667)
    lp.run_primal_dual();

    lp.add_dual_variable(6, {5, 4, 3});
    run_test(lp); // 6, (0, 1.5, 0)
    lp.run_primal_dual();

    lp.add_primal_variable(7, {4, 1, 3, 1});
    run_test(lp); // 26.5, (0, 0.125, 0, 4.125)
    lp.run_primal_dual();
}

int main() {
    RUN_BLOCK(unit_test_simplex());
    return 0;
}
