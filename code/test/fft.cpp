#include "test_utils.hpp"
#include "numeric/fft.hpp"

template <typename Num = int64_t>
void stress_test_fft_multiply(int V) {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 100'000, runs) {
        print_time(now, 20s, "stress fft ({} runs)", runs);

        int A = rand_unif<int>(0, 1000);
        int B = rand_unif<int>(0, 1000);
        auto a = rands_unif<int, Num>(A, -V, V);
        auto b = rands_unif<int, Num>(B, -V, V);
        auto c = fft::fft_multiply(a, b);
        auto d = fft::naive_multiply(a, b);

        assert(c == d);
    }
}

template <typename Num = int64_t>
void speed_test_fft_multiply(Num V) {
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
        START_ACC(fft);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 10000, runs) {
            print_time(now, runtime, "speed fft A={} B={} ({} runs)", A, B, runs);

            auto a = rands_unif<int, Num>(A, -V, V);
            auto b = rands_unif<int, Num>(B, -V, V);

            ADD_TIME_BLOCK(fft) { fft::fft_multiply(a, b); }
        }

        table[{A, B}] = FORMAT_EACH(fft, runs);
    }

    print_time_table(table, "FFT multiply");
}

int main() {
    RUN_BLOCK(stress_test_fft_multiply<int>(1000));
    RUN_BLOCK(stress_test_fft_multiply<int64_t>(100'000));
    RUN_BLOCK(speed_test_fft_multiply<int64_t>(100'000));
    return 0;
}
