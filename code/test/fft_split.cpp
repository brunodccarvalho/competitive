#include "test_utils.hpp"
#include "numeric/fft.hpp"

void stress_test_fft_split_multiply() {
    constexpr int N = 1000;
    constexpr int V = 40'000'000;
    static_assert(__int128_t(N) * V * V <= LONG_MAX / 4);

    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 100'000, runs) {
        print_time(now, 20s, "stress fft-split ({} runs)", runs);

        int A = rand_unif<int>(1, N);
        int B = rand_unif<int>(1, N);
        auto a = rands_unif<long>(A, -V, V);
        auto b = rands_unif<long>(B, -V, V);
        auto c = fft::fft_split_multiply(a, b);
        auto d = fft::naive_multiply(a, b);

        assert(c == d);
    }
}

void speed_test_fft_split_multiply() {
    vector<int> As = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};
    vector<int> Bs = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};

    vector<pair<int, int>> inputs;
    for (int A : As) {
        for (int B : Bs) {
            inputs.push_back({A, B});
        }
    }

    const auto runtime = 120'000ms / (As.size() * Bs.size());
    map<pair<int, int>, string> table;

    for (auto [A, B] : inputs) {
        START_ACC(fft_split);
        long V = sqrt(LONG_MAX / 4 / (A + B));

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 10000, runs) {
            print_time(now, runtime, "speed fft-split A={} B={} ({} runs)", A, B, runs);

            auto a = rands_unif<long>(A, -V, V);
            auto b = rands_unif<long>(B, -V, V);

            ADD_TIME_BLOCK(fft_split) { fft::fft_split_multiply(a, b); }
        }

        table[{A, B}] = FORMAT_EACH(fft_split, runs);
    }

    print_time_table(table, "FFT-split multiply");
}

int main() {
    RUN_BLOCK(stress_test_fft_split_multiply());
    RUN_BLOCK(speed_test_fft_split_multiply());
    return 0;
}
