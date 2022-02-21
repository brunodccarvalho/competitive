#include "test_utils.hpp"
#include "numeric/modnum.hpp"
#include "linear/berlekamp_massey.hpp"
#include "lib/lr.hpp"

constexpr int M = 998244353;
using num = modnum<M>;
using namespace polymath;

void stress_test_berlekamp_massey() {
    int shorter = 0;
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (30s, now, 50'000, runs) {
        print_time(now, 30s, "stress BM ({} runs) ({} shorter)", runs, shorter);
        int n = rand_unif<int>(1, 100);
        int m = rand_unif<int>(2 * n, 7 * n);

        auto rec = rand_recurrence<num>(n);
        auto x = rand_poly<num>(n);
        extend_recurrence_inplace<num>(m, x, rec);

        assert(size(x) == m && size(rec) == n);

        auto ans = berlekamp_massey(x);
        if (ans != rec) {
            assert(size(ans) <= size(rec));
            assert(verify_recurrence(x, rec));
            shorter++;
        }
    }
}

void stress_test_kitamasa() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (30s, now, 50'000, runs) {
        print_time(now, 30s, "stress kitamasa ({} runs)", runs);
        int n = rand_unif<int>(1, 40);
        int m = rand_unif<int>(2 * n, 100 * n);
        int k = rand_unif<int>(n, 2 * n);

        auto rec = rand_recurrence<num>(n);
        auto x = rand_poly<num>(n);
        extend_recurrence_inplace<num>(m, x, rec);
        auto y = shrunk(x, k);

        assert(size(x) == m && size(y) == k && size(rec) == n);

        for (int i = 0; i < m; i++) {
            assert(kitamasa(y, rec, i) == x[i]);
        }
    }
}

void speed_test_berlekamp_massey() {
    vector<int> Ns = {100, 300, 600, 1000, 1800, 3000, 5000, 8000, 12000, 20000};
    vector<int> Ms = {2000, 3000, 5000, 10000, 20000, 30000, 40000};

    vector<pair<int, int>> inputs;
    for (int N : Ns) {
        for (int M : Ms) {
            if (2 * N <= M) {
                inputs.push_back({N, M});
            }
        }
    }

    auto runtime = 120'000ms / inputs.size();
    map<pair<int, int>, stringable> table;

    for (auto [N, M] : inputs) {
        START_ACC(bm);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 10'000, runs) {
            print_time(now, runtime, "speed BM N={} M={} ({} runs)", N, M, runs);

            auto rec = rand_recurrence<num>(N);
            auto x = rand_poly<num>(N);
            extend_recurrence_inplace<num>(M, x, rec);

            ADD_TIME_BLOCK(bm) { berlekamp_massey(x); }
        }

        table[{M, N}] = FORMAT_EACH(bm, runs);
    }

    print_time_table(table, "Berlekamp-Massey");
}

void speed_test_kitamasa() {
    vector<int> Ns = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000};
    vector<double> Ks = {1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15};

    vector<pair<int, uint64_t>> inputs;
    for (int N : Ns) {
        for (auto K : Ks) {
            inputs.push_back({N, uint64_t(K)});
        }
    }

    const auto runtime = 300'000ms / inputs.size();
    map<pair<stringable, int>, stringable> table;

    for (auto [N, K] : inputs) {
        START_ACC(kita);
        string Kstr = format("{:.0e}", 1.0 * K);

        auto rec = rand_recurrence<num>(N);
        auto x = rand_poly<num>(N);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 10'000, runs) {
            print_time(now, runtime, "speed KM N={} K={} ({} runs)", N, Kstr, runs);

            ADD_TIME_BLOCK(kita) {
                int64_t k = rand_unif<int64_t>(K, 1.1 * K);
                kitamasa(x, rec, k);
            }
        }

        table[{Kstr, N}] = FORMAT_EACH(kita, runs);
    }

    print_time_table(table, "Kitamasa Kth Term");
}

int main() {
    RUN_BLOCK(stress_test_kitamasa());
    RUN_BLOCK(speed_test_kitamasa());
    RUN_BLOCK(stress_test_berlekamp_massey());
    RUN_BLOCK(speed_test_berlekamp_massey());
    return 0;
}
