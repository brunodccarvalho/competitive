#include "test_utils.hpp"
#include "algo/optimization.hpp"

template <typename V, typename Dn, typename CostFn>
auto solve_1d1d_naive(int N, V zero, Dn&& dn, CostFn&& costfn) {
    static constexpr V inf = numeric_limits<V>::max();
    vector<V> E(N, inf), D(N);
    E[0] = zero;
    D[0] = dn(E[0], 0);

    for (int i = 1; i < N; i++) {
        for (int j = 0; j < i; j++) {
            E[i] = min(E[i], D[j] + costfn(j, i));
        }
        D[i] = dn(E[i], i);
    }

    return E;
}

void stress_test_solve_1d1d_concave() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (5s, now, 100'000, runs) {
        print_time(now, 5s, "stress test 1d1d concave ({} runs)\r", runs);
        int N = rand_unif<int>(1, 100);
        int A = rand_unif<int>(-3, 0);
        int B = rand_unif<int>(-100, 100);
        int C = rand_unif<int>(-300, 300);
        auto d = rands_unif<int>(N, -900, 900);
        auto costfn = [&](int j, int i) {
            return A * (i - j) * (i - j) + B * (i - j) + C;
        };
        auto dn = [&](int64_t v, int i) { return v + d[i]; };

        auto fast = solve_1d1d_concave(N, 0L, dn, costfn);
        auto naive = solve_1d1d_naive(N, 0L, dn, costfn);
        assert(naive == fast);
    }
}

void stress_test_solve_1d1d_convex() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (5s, now, 100'000, runs) {
        print_time(now, 5s, "stress test 1d1d convex ({} runs)\r", runs);
        int N = rand_unif<int>(1, 100);
        int A = rand_unif<int>(0, 3);
        int B = rand_unif<int>(-100, 100);
        int C = rand_unif<int>(-300, 300);
        auto d = rands_unif<int>(N, -900, 900);
        auto costfn = [&](int j, int i) {
            return A * (i - j) * (i - j) + B * (i - j) + C;
        };
        auto dn = [&](int64_t v, int i) { return v + d[i]; };

        auto fast = solve_1d1d_convex(N, 0L, dn, costfn);
        auto naive = solve_1d1d_naive(N, 0L, dn, costfn);
        assert(naive == fast);
    }
}

int main() {
    RUN_BLOCK(stress_test_solve_1d1d_concave());
    RUN_BLOCK(stress_test_solve_1d1d_convex());
    return 0;
}

// A(a-j)(a-j) + B(a-j) - A(b-j)(b-j) - B(b-j)
// A(a^2 + j^2 - 2aj - b^2 -j^2 + 2bj) + B(a-b)
// A(a^2 - b^2 - 2(a-b)j) + B(a-b)
// A(a^2 - b^2) + (a-b)(Aj+B-2)
// A(a^2 - b^2) + (a-b)(Aj+B-2) >= A(a^2 - b^2) + (a-b)(Ai+B-2)
// (a-b)Aj >= (a-b)Ai  true for A<=0
