#include "test_utils.hpp"
#include "numeric/fft.hpp"
#include "lib/anynum.hpp"

constexpr int MOD = 1'000'000'007;
using num = modnum<MOD>;

template <typename T>
auto rands_grav_or_wide(int N, T a, T b, int off) {
    return cointoss(0.5) ? rands_grav<T>(N, a, b, off) : rands_wide<T>(N, a, b, off);
}

void sqrt_merge(int Q, vector<int>& a, const vector<int>& b) {
    for (int i = 0, N = a.size(); i < N; i++) {
        a[i] = Q * a[i] + b[i];
        assert(0 <= a[i] && a[i] < MOD);
    }
}

void stress_test_fft_multiply_mod() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 100'000, runs) {
        print_time(now, 20s, "stress ntt-split ({} runs)", runs);

        int A = rand_unif<int>(0, 1000);
        int B = rand_unif<int>(0, 1000);
        auto a = rands_grav_or_wide<int>(A, 0, MOD - 1, +5);
        auto b = rands_grav_or_wide<int>(B, 0, MOD - 1, +5);

        auto c = fft::fft_multiply(MOD, a, b);
        auto d = fft::naive_multiply(MOD, a, b);
        assert(c == d);
    }
}

void killer_test_fft_multiply_mod() {
    using large_complex = fft::my_complex<long double>;
    int Q = sqrt(MOD);
    int errors = 0;

    LOOP_FOR_DURATION_OR_RUNS_TRACKED (300s, now, 200, runs) {
        print_time(now, 300s, "killer ntt-split {:2} errors ({} runs)", errors, runs);

        int A = 1 << 18;
        int B = 1 << 18;
        auto a = rands_grav_or_wide<int>(A, 0, Q - 1, +5);
        auto b = rands_grav_or_wide<int>(B, 0, Q - 1, +5);
        sqrt_merge(Q, a, rands_grav_or_wide<int>(A, 0, Q - 1, +5));
        sqrt_merge(Q, b, rands_grav_or_wide<int>(B, 0, Q - 1, +5));

        auto c = fft::fft_multiply<int64_t, fft::default_complex>(MOD, a, b);
        auto d = fft::fft_multiply<int64_t, large_complex>(MOD, a, b);
        errors += c != d;
    }

    // Choose always a[i]:              fails 170/200 cases
    // Choose always a[i] - mod:        fails 120/200 cases
    // Choose min abs(a[i], a[i]-mod):  fails   0/100 cases
    // Cointoss a[i] and a[i]-mod:      fails   0/100 cases
    printcl("errors: {}\n", errors);
}

void speed_test_fft_multiply_mod() {
    vector<int> As = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};
    vector<int> Bs = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};

    vector<pair<int, int>> inputs;
    for (int A : As) {
        for (int B : Bs) {
            inputs.push_back({A, B});
        }
    }

    const auto runtime = 120'000ms / (As.size() * Bs.size());
    map<tuple<int, int, stringable>, string> table;

    for (auto [A, B] : inputs) {
        START_ACC2(fft_mod, fft_modnum);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 10000, runs) {
            print_time(now, runtime, "speed ntt-split A={} B={} ({} runs)", A, B, runs);

            auto a = rands_unif<int>(A, 0, MOD - 1);
            auto b = rands_unif<int>(B, 0, MOD - 1);
            vector<num> c(begin(a), end(a));
            vector<num> d(begin(b), end(b));

            ADD_TIME_BLOCK(fft_mod) { fft::fft_multiply(MOD, a, b); }
            ADD_TIME_BLOCK(fft_modnum) { fft::fft_multiply(c, d); }
        }

        table[{A, B, "mod"}] = FORMAT_EACH(fft_mod, runs);
        table[{A, B, "modnum"}] = FORMAT_EACH(fft_modnum, runs);
    }

    print_time_table(table, "NTT-split multiply");
}

int main() {
    RUN_BLOCK(killer_test_fft_multiply_mod());
    RUN_BLOCK(stress_test_fft_multiply_mod());
    RUN_BLOCK(speed_test_fft_multiply_mod());
    return 0;
}
