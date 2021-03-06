#include "test_utils.hpp"
#include "../numeric/fft.hpp"
#include "../lib/anynum.hpp"

using num = modnum<998244353>;

void stress_test_ntt_multiply(int N = 1000, int V = 100000) {
    intd distn(0, N);
    int errors = 0;

    LOOP_FOR_DURATION_OR_RUNS_TRACKED (4s, now, 100'000, runs) {
        print_time(now, 4s, "stress test ntt multiply");

        int A = distn(mt), B = distn(mt);
        auto a = uniform_gen_many<int, num>(A, 0, V);
        auto b = uniform_gen_many<int, num>(B, 0, V);
        auto c = fft::ntt_multiply(a, b);
        auto d = fft::naive_multiply(a, b);

        errors += c != d;
    }

    if (errors > 0) {
        double percent = 100.0 * errors / runs;
        printcl("{} ({:.1f}%) ERRORS (N<={},|V|<={})\n", errors, percent, N, V);
    }
}

void breakeven_test_ntt_multiply() {
    if (fft::MODNUM_BREAKEVEN)
        return;

    constexpr int V = 10000;
    vector<size_t> fft_time{1}, naive_time{1};

    // add -1 when fft is better, +1 when naive is better, run until delta is stop
    const int window = 40, stop = -30;
    int delta = 0, window_len = 0;
    int N = 0;

    while (delta > stop) {
        printcl("breakeven ntt multiply N={} window={}", N, delta);
        N++;

        START_ACC(fft);
        START_ACC(naive);

        LOOP_FOR_DURATION_OR_RUNS (30ms, 10000) {
            auto a = uniform_gen_many<int, num>(N, 0, V);
            auto b = uniform_gen_many<int, num>(N, 0, V);

            START(fft);
            auto c = fft::ntt_multiply(a, b);
            ADD_TIME(fft);

            START(naive);
            auto d = fft::naive_multiply(a, b);
            ADD_TIME(naive);
        }

        fft_time.push_back(TIME_US(fft));
        naive_time.push_back(TIME_US(naive));

        delta += fft_time[N] < naive_time[N] ? -1 : 1;
        window_len++;
        if (window_len > window) {
            delta -= fft_time[N - window] < naive_time[N - window] ? -1 : 1;
            window_len--;
        }
    }

    printcl("breakeven: N={}\n", N - window / 2);
}

void unit_test_ntt() {
    vector<num> a = {123412, 315312, 644121};
    vector<num> b = {123512, 52319023, 123512};
    auto c = fft::ntt_multiply(a, b);
    auto d = fft::naive_multiply(a, b);
    print("c: {}\n", c);
    print("d: {}\n", d);
}

int main() {
    RUN_SHORT(unit_test_ntt());
    RUN_BLOCK(breakeven_test_ntt_multiply());
    RUN_SHORT(stress_test_ntt_multiply());
    return 0;
}
