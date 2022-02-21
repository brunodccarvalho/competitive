#include "test_utils.hpp"
#include "numeric/fft.hpp"

using num = modnum<998244353>;

void stress_test_ntt_multiply() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 100'000, runs) {
        print_time(now, 20s, "stress ntt ({} runs)", runs);

        const int V = 100'000;
        int A = rand_unif<int>(0, 1000);
        int B = rand_unif<int>(0, 1000);
        auto a = rands_unif<int, num>(A, -V, V);
        auto b = rands_unif<int, num>(B, -V, V);
        auto c = fft::ntt_multiply(a, b);
        auto d = fft::naive_multiply(a, b);

        assert(c == d);
    }
}

void speed_test_ntt_multiply() {
    const int V = 1'000'000;
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
        START_ACC(ntt);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 10'000, runs) {
            print_time(now, runtime, "speed ntt A={} B={} ({} runs)", A, B, runs);

            auto a = rands_unif<int, num>(A, -V, V);
            auto b = rands_unif<int, num>(B, -V, V);

            ADD_TIME_BLOCK(ntt) { fft::ntt_multiply(a, b); }
        }

        table[{A, B}] = FORMAT_EACH(ntt, runs);
    }

    print_time_table(table, "NTT multiply");
}

int main() {
    RUN_BLOCK(stress_test_ntt_multiply());
    RUN_BLOCK(speed_test_ntt_multiply());
    return 0;
}
