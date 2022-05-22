#include "test_utils.hpp"
#include "../algo/knapsack.hpp"

auto naive_min_plus(vector<int> a, vector<int> b) {
    int N = a.size(), M = b.size();
    const int inf = INT_MAX / 3;
    vector<int> c(N + M - 1, inf);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            c[i + j] = min(c[i + j], a[i] + b[j]);
        }
    }
    return c;
}

auto random_convex_vector(int N) {
    vector<int> A(N);
    int G = rand_unif<int>(-3, +3);
    int D = rand_unif<int>(-N * N, N);
    for (int i = 1; i < N; i++) {
        D += rand_wide<int>(0, 4 * N, G);
        A[i] = A[i - 1] + D;
    }
    return A;
}

void stress_test_smawk() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (5s, now, 100'000, runs) {
        print_time(now, 5s, "stress smawk ({} runs)", runs);
        int N = rand_unif<int>(1, 30);
        int M = rand_unif<int>(1, 30);
        auto a = rands_unif<int>(N, -1e7, 1e7);
        auto b = random_convex_vector(M);
        const int inf = INT_MAX / 3;
        auto f = [&](int r, int c) {
            return r >= c && r - c < M ? a[c] + b[r - c] : inf;
        };
        auto cols = min_smawk(f, N + M - 1, N);
        auto c = naive_min_plus(a, b);
        vector<int> d(N + M - 1);
        for (int r = 0; r < N + M - 1; r++) {
            assert(r == 0 || cols[r - 1] <= cols[r]);
            d[r] = f(r, cols[r]);
        }
        assert(c == d);
    }
}

int main() {
    RUN_BLOCK(stress_test_smawk());
    return 0;
}
