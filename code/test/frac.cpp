#include "test_utils.hpp"
#include "numeric/frac.hpp"
#include "numeric/quot.hpp"

using F = frac<int64_t>;
using Q = quot<int64_t>;

void speed_test_rational_comparison() {
    using int128_t = __int128_t;
    using ld = long double;

    // Let vs = {{a,b,c,d}...}. We will compare a/b with c/d
    constexpr int32_t mx = INT32_MAX / 2;

    const int N = 10'000'000;
    vector<array<int, 4>> vs(N);
    for (int i = 0; i < N; i++) {
        print_regular(i, N, 10000, "generating");
        int a = rand_wide<int>(-mx, mx, -3);
        int b = rand_wide<int>(0, mx, -15);
        int c = rand_wide<int>(-mx, mx, -3);
        int d = rand_wide<int>(0, mx, -15);
        if (b != 0 && cointoss(0.1))
            a = 0;
        else if (a != 0 && cointoss(0.1))
            b = 0;
        if (d != 0 && cointoss(0.1))
            c = 0;
        else if (c != 0 && cointoss(0.1))
            d = 0;
        vs[i] = {a, b, c, d};
    }

    int real_count = 0;
    TIME_BLOCK(double) {
        for (auto [a, b, c, d] : vs) {
            double x = double(a) / b, y = double(c) / d;
            real_count += x < y;
        }
    }
    TIME_BLOCK(long_double) {
        int count = 0;
        for (auto [a, b, c, d] : vs) {
            ld x = ld(a) / b, y = ld(c) / d;
            count += x < y;
        }
        assert(count == real_count);
    }
    TIME_BLOCK(quot32) {
        int count = 0;
        for (auto [a, b, c, d] : vs) {
            quot<int32_t, false> x(a, b), y(c, d);
            count += x < y;
        }
        assert(count == real_count);
    }
    TIME_BLOCK(quot64) {
        int count = 0;
        for (auto [a, b, c, d] : vs) {
            quot<int64_t, true> x(a, b), y(c, d);
            count += x < y;
        }
        assert(count == real_count);
    }
    TIME_BLOCK(quot128) {
        int count = 0;
        for (auto [a, b, c, d] : vs) {
            quot<int128_t, true> x(a, b), y(c, d);
            count += x < y;
        }
        assert(count == real_count);
    }
    TIME_BLOCK(frac32) {
        int count = 0;
        for (auto [a, b, c, d] : vs) {
            frac<int32_t, false> x(a, b), y(c, d);
            count += x < y;
        }
        assert(count == real_count);
    }
    TIME_BLOCK(frac64) {
        int count = 0;
        for (auto [a, b, c, d] : vs) {
            frac<int64_t, true> x(a, b), y(c, d);
            count += x < y;
        }
        assert(count == real_count);
    }
}

void stress_test_quot() {
    int N = 50;
    for (int a = -N; a <= N; a++) {
        for (int b = 0; b <= N; b++) {
            if (a == 0 && b == 0)
                continue;

            for (int c = -N; c <= N; c++) {
                for (int d = 0; d <= N; d++) {
                    if (c == 0 && d == 0)
                        continue;

                    Q x(a, b), y(c, d);
                    Q op1(a * d + b * c, b * d);
                    Q op2(a * d - b * c, b * d);
                    Q op3(a * c, b * d);
                    Q op4(a * d, b * c);

                    assert((Q::compare(x, y) < 0) == (double(a) / b < double(c) / d));
                    assert(Q::undefined(op1) || op1 == x + y);
                    assert(Q::undefined(op2) || op2 == x - y);
                    assert(d == 0 || Q::undefined(op3) || op3 == x * y);
                    assert(b == 0 || Q::undefined(op4) || op4 == x / y);
                }
            }
        }
    }
}

void stress_test_frac() {
    int N = 50;
    for (int a = -N; a <= N; a++) {
        for (int b = 0; b <= N; b++) {
            if (a == 0 && b == 0)
                continue;

            for (int c = -N; c <= N; c++) {
                for (int d = 0; d <= N; d++) {
                    if (c == 0 && d == 0)
                        continue;

                    F x(a, b), y(c, d);
                    F op1(a * d + b * c, b * d);
                    F op2(a * d - b * c, b * d);
                    F op3(a * c, b * d);
                    F op4(a * d, b * c);

                    assert((F::compare(x, y) < 0) == (double(a) / b < double(c) / d));
                    assert(F::undefined(op1) || op1 == x + y);
                    assert(F::undefined(op2) || op2 == x - y);
                    assert(d == 0 || F::undefined(op3) || op3 == x * y);
                    assert(b == 0 || F::undefined(op4) || op4 == x / y);
                }
            }
        }
    }
}

void unit_test_gcd() {
    assert(F(93, 31) == F(3, 1));
    assert(F(7, -19) == F(-7, 19));
    assert(F(-74, -4) == F(37, 2));
    assert(F(3, 0) == F(1, 0));
    assert(F(-73, 0) == F(-1, 0));
    assert(F(-1, 0) < LONG_MIN);
    assert(F(1, 0) > LONG_MAX);
    assert(F(7, -3) == F(-7, 3));
    assert(floor(F(7, 3)) == 2L);
    assert(floor(F(6, 3)) == 2L);
    assert(floor(F(-7, 3)) == -3L);
    assert(floor(F(-6, 3)) == -2L);
    assert(ceil(F(7, 3)) == 3L);
    assert(ceil(F(6, 3)) == 2L);
    assert(ceil(F(-7, 3)) == -2L);
    assert(ceil(F(-6, 3)) == -2L);
}

void unit_test_ops() {
    assert(F(2, 3) + F(3, 4) == F(17, 12));
    assert(abs(F(2, 3) - F(3, 4)) == F(1, 12));
    assert(F(3, 7) * F(7, 8) == F(3, 8));
    assert(F(1, 2) / F(1, 7) == F(7, 2));
    assert(3L / F(7, 2) == F(6, 7));
    assert(4L * F(7, 9) == F(28, 9));
    assert(2L + F(7, 9) == F(25, 9));
    assert(1L - F(7, 9) == F(2, 9));
    assert(2L % F(7, 9) == F(4, 9));
    assert(F(9, 2) / 3L == F(3, 2));
    assert(F(9, 2) / 4L == F(9, 8));

    assert(F(7, 3) % 2L == F(1, 3));
    assert(F(29, 9) % F(6, 7) == F(41, 63));
    assert(F(29, 9) % F(-6, 7) == F(41, 63));
    assert(F(-29, 9) % F(6, 7) == F(-41, 63));
    assert(F(-29, 9) % F(-6, 7) == F(-41, 63));
}

void unit_test_read() {
    assert(stofrac<F>("-123") == -123);
    assert(stofrac<F>("-123.00") == -123);
    assert(stofrac<F>("-123.5") == F(-247, 2));
    assert(stofrac<F>(" -247/2") == F(-247, 2));
    assert(stofrac<F>("  -494/4") == F(-247, 2));
    assert(stofrac<F>(" -494/-4") == -494);
    assert(stofrac<F>("+7/8") == F(7, 8));
    assert(stofrac<F>(" 123/456") == F(123, 456));
    assert(stofrac<F>(" 123/0") == F(1, 0));
    assert(stofrac<F>(" 0/123 ") == 0);
    assert(stofrac<F>("  1.5") == F(3, 2));
    assert(stofrac<F>(" 1.") == 1);
    assert(stofrac<F>(" .1") == F(1, 10));
    assert(stofrac<F>(" -.100") == F(-1, 10));
    assert(stofrac<F>(" -712412.71231") == -F(71241271231, 100000));
    assert(stofrac<F>(" -712412/71231") == -F(712412, 71231));
}

void stress_test_quot_binary_search() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (10s, now, 100'000'000, runs) {
        print_time(now, 10s, "stress test frac binary search ({} runs)", runs);

        const int64_t N = 100'000;
        int64_t n = rand_unif<int64_t>(-N, N);
        int64_t d = rand_unif<int64_t>(1, N);
        F ans(n, d);
        auto pred = [&](const auto& f) { return f < ans ? 0 : ans < f ? 2 : 1; };
        F got = frac_binary_search(pred);
        assert(ans == got);
    }
}

void stress_test_frac_binary_search() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (10s, now, 100'000'000, runs) {
        print_time(now, 10s, "stress test quot binary search ({} runs)", runs);

        const int64_t N = 100'000;
        int64_t n = rand_unif<int64_t>(-N, N);
        int64_t d = rand_unif<int64_t>(1, N);
        Q ans(n, d);
        auto pred = [&](const auto& f) { return f < ans ? 0 : ans < f ? 2 : 1; };
        Q got = quot_binary_search(pred);
        assert(ans == got);
    }
}

void stress_test_frac_bounded_search() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (10s, now, 100'000'000, runs) {
        print_time(now, 10s, "stress test frac upper bound ({} runs)", runs);

        const int64_t N = 100'000;
        int64_t n = rand_unif<int64_t>(-N, N);
        int64_t d = rand_unif<int64_t>(1, N);
        int64_t b = rand_wide<int64_t>(1, N, -2);
        F ans(n, d);
        auto pred = [&](const auto& f) { return f >= ans; };
        F got = frac_bounded_search(pred, N, b);
        assert(ans <= got && got - ans < F(1, b));
    }
}

void stress_test_quot_bounded_search() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (10s, now, 100'000'000, runs) {
        print_time(now, 10s, "stress test quot upper bound ({} runs)", runs);

        const int64_t N = 100'000;
        int64_t n = rand_unif<int64_t>(-N, N);
        int64_t d = rand_unif<int64_t>(1, N);
        int64_t b = rand_wide<int64_t>(1, N, -2);
        Q ans(n, d);
        auto pred = [&](const auto& f) { return f >= ans; };
        Q got = quot_bounded_search(pred, N, b);
        assert(ans <= got && got - ans < Q(1, b));
    }
}

int main() {
    RUN_SHORT(unit_test_gcd());
    RUN_SHORT(unit_test_ops());
    RUN_SHORT(unit_test_read());
    RUN_BLOCK(stress_test_frac_binary_search());
    RUN_BLOCK(stress_test_quot_binary_search());
    RUN_BLOCK(stress_test_frac_bounded_search());
    RUN_BLOCK(stress_test_quot_bounded_search());
    return 0;
}
