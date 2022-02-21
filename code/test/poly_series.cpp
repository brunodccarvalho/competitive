#include "test_utils.hpp"
#include "numeric/fft.hpp"
#include "numeric/polynomial.hpp"

using namespace polymath;
using num = modnum<998244353>;

void speed_test_polynomial() {
    vector<int> Ns = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};

    vector<int> inputs = Ns;

    const int FIXED = 5;
    const auto runtime = 1200'000ms / inputs.size();
    map<pair<stringable, int>, stringable> table;

    for (int N : inputs) {
        START_ACC5(inverse, exp, log, pow2_5, pow2_15);
        START_ACC2(div, mod);
        START_ACC5(faulhaber, st1stn, st1stk, st2ndn, st2ndk);
        START_ACC5(bell, partitions, bernoulli, eulerian, alternating);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 300, runs) {
            print_time(now, runtime, "speed polynomial N={} ({} runs)", N, runs);

            auto poly = rands_unif<int, num>(N, 1, num::MOD - 1);
            auto div = rands_unif<int, num>(N / 2, 1, num::MOD - 1);
            poly[0] = div[0] = 1;

            ADD_TIME_BLOCK(inverse) { inverse(poly); }
            ADD_TIME_BLOCK(exp) { poly[0] = 0, exp(poly); }
            ADD_TIME_BLOCK(log) { poly[0] = 1, log(poly); }
            ADD_TIME_BLOCK(pow2_5) { pow(poly, 1 << 5); }
            ADD_TIME_BLOCK(pow2_15) { pow(poly, 1 << 15); }
            ADD_TIME_BLOCK(div) { poly / div; }
            ADD_TIME_BLOCK(mod) { poly % div; }
        }

        for (int i = 0; i < FIXED; i++) {
            ADD_TIME_BLOCK(faulhaber) { series::faulhaber<num>(N, 1 << 18); }
            ADD_TIME_BLOCK(st1stn) { series::stirling_1st<num>(N); }
            ADD_TIME_BLOCK(st1stk) { series::stirling_1st<num>(N, N / 2); }
            ADD_TIME_BLOCK(st2ndn) { series::stirling_2nd<num>(N); }
            ADD_TIME_BLOCK(st2ndk) { series::stirling_2nd<num>(N, N / 2); }
            ADD_TIME_BLOCK(bell) { series::bell<num>(N); }
            ADD_TIME_BLOCK(bernoulli) { series::bernoulli<num>(N); }
            ADD_TIME_BLOCK(partitions) { series::partitions<num>(N); }
            ADD_TIME_BLOCK(eulerian) { series::eulerian<num>(N); }
            ADD_TIME_BLOCK(alternating) { series::euler_alternating<num>(N); }
        }

        table[{"inverse", N}] = FORMAT_EACH(inverse, runs);
        table[{"exp", N}] = FORMAT_EACH(exp, runs);
        table[{"log", N}] = FORMAT_EACH(log, runs);
        table[{"pow2_5", N}] = FORMAT_EACH(pow2_5, runs);
        table[{"pow2_15", N}] = FORMAT_EACH(pow2_15, runs);
        table[{"div", N}] = FORMAT_EACH(div, runs);
        table[{"mod", N}] = FORMAT_EACH(mod, runs);
        table[{"faulhaber", N}] = FORMAT_EACH(faulhaber, FIXED);
        table[{"st1stn", N}] = FORMAT_EACH(st1stn, FIXED);
        table[{"st1stk", N}] = FORMAT_EACH(st1stk, FIXED);
        table[{"st2ndn", N}] = FORMAT_EACH(st2ndn, FIXED);
        table[{"st2ndk", N}] = FORMAT_EACH(st2ndk, FIXED);
        table[{"bell", N}] = FORMAT_EACH(bell, FIXED);
        table[{"partitions", N}] = FORMAT_EACH(partitions, FIXED);
        table[{"bernoulli", N}] = FORMAT_EACH(bernoulli, FIXED);
        table[{"eulerian", N}] = FORMAT_EACH(eulerian, FIXED);
        table[{"alternating", N}] = FORMAT_EACH(alternating, FIXED);
    }

    print_time_table(table, "Polynomial");
}

void unit_test_polyseries() {
    println("falling: {}", series::falling_factorial<num>(15));
    println("rising: {}", series::rising_factorial<num>(15));
    println("falling: {}", series::falling_factorial<num>(16));
    println("rising: {}", series::rising_factorial<num>(16));

    println("stirling 1st n=9: {}", series::stirling_1st<num>(9));
    println("stirling 1st k=5: {}", series::stirling_1st<num>(9, 5));
    println("stirling 2nd n=9: {}", series::stirling_2nd<num>(9));
    println("stirling 2nd k=5: {}", series::stirling_2nd<num>(9, 5));

    println("A(n=9,k): {}", series::eulerian<num>(9));
    println("E(n): {}", series::euler_alternating<num>(15));

    println("faulhaber, m=4: {}", series::faulhaber<num>(8, 4));

    println("bell: {}", series::bell<num>(15));
    println("partitions: {}", series::partitions<num>(15));

    println("sine: {}", series::sine<num>(15));
    println("cosine: {}", series::cosine<num>(15));
    println("secant: {}", series::secant<num>(15));
    println("tangent: {}", series::tangent<num>(15));
}

int main() {
    RUN_BLOCK(unit_test_polyseries());
    RUN_BLOCK(speed_test_polynomial());
    return 0;
}
