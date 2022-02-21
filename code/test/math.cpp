#include "test_utils.hpp"
#include "numeric/math.hpp"
#include "numeric/modnum.hpp"
#include "algo/floor_sum.hpp"

void stress_test_floor_sum() {
    vector<int> ns = {1, 2, 3, 4, 5, 37, 89, 913, 1024, 7302};
    vector<long> ms = {2, 3, 4, 5, 7, 13, 40, 73, 431, 999, 1000, 50000, 150000, 3000000};
    vector<long> as = {1, 2, 3, 4, 5, 17, 28, 41, 99, 713, 4123, 81423, 1024123, 7612312};
    vector<long> bs = {1, 2, 3, 4, 5, 19, 31, 43, 73, 314, 6712, 54261, 3124421, 9617236};

    auto run = [&](long m, long a, long b) {
        int N = *max_element(begin(ns), end(ns));
        vector<long> sum(N + 1);

        for (int i = 0; i < N; i++) {
            sum[i + 1] = sum[i] + (a * i + b) / m;
        }

        for (int n : ns) {
            assert(sum[n] == floor_sum(n, m, a, b));
        }

        printcl("floor sum {} {} {} OK", m, a, b);
    };

    for (long m : ms) {
        for (long a : as) {
            for (long b : bs) {
                run(m, a, b);
            }
        }
    }
}

void unit_test_modlog() {
    assert(modlog(4, 2, 7) == 2);
    assert(modlog(4, 2, 9) == -1);
    assert(modlog(3, 1, 13) == 3);
    assert(modlog(7, 2, 19) == -1);
    assert(modlog(9, 4, 19) == 7);
    assert(modlog(6, 1, 31) == 6);

    int hit = 0, miss = 0;
    for (long p : {7, 31, 51, 73, 97, 103, 111, 131, 151, 219, 250}) {
        for (long a = 1; a < p; a++) {
            for (long b = 1; b < p; b++) {
                long x = modlog(a, b, p);
                x != -1 ? hit++ : miss++;
                if (x != -1) {
                    assert(modpow(a, x, p) == b);
                }
            }
        }
    }
    print("modlog: hit: {} | miss: {}\n", hit, miss);
}

void unit_test_modsqrt() {
    assert(modsqrt(41, 61) == 38 || modsqrt(41, 61) == 23);
    assert(modpow(38, 2, 61) == 41 && modpow(23, 2, 61) == 41);

    int hit = 0, miss = 0;
    for (long p : {7, 31, 73, 97, 103, 131, 151, 251, 313, 571, 787, 991}) {
        for (long a = 1; a < p; a++) {
            long x = modsqrt(a, p);
            x != -1 ? hit++ : miss++;
            if (x != -1) {
                assert(modpow(x, 2, p) == a && modpow(-x, 2, p) == a);
            }
        }
    }
    assert(hit == miss);
}

#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

void stress_test_modnum() {
    constexpr int mod = 998244353;
    using num = modnum<mod>;

    constexpr int N = 30'000, M = 30'000;
    vector<uint> a(N), b(M);
    for (uint i = 0; i < N; i++) {
        a[i] = rand_unif<uint>(1, mod - 1);
    }
    for (uint i = 0; i < M; i++) {
        b[i] = rand_unif<uint>(1, mod - 1);
    }

    num ans = 0;
    TIME_BLOCK(modnum) {
        vector<num> am(N), bm(M);
        for (int i = 0; i < N; i++)
            am[i] = num(a[i]);
        for (int j = 0; j < M; j++)
            bm[j] = num(b[j]);

        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                ans += am[i] * bm[j];
        for (int i = 0; i < N; i++)
            ans *= am[i];
        for (int j = 0; j < M; j++)
            ans *= bm[j];
    }

    TIME_BLOCK(1LL) {
        uint d = 0;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                d += (1ULL * a[i] * b[j]) % mod;
                if (d >= mod)
                    d -= mod;
            }
        }
        for (int i = 0; i < N; i++) {
            d = 1LL * d * a[i] % mod;
        }
        for (int j = 0; j < M; j++) {
            d = 1LL * d * b[j] % mod;
        }
        assert(ans == d);
    }

    TIME_BLOCK(smul) {
        constexpr double s = (1.0) / mod;
        uint d = 0;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                auto n = 1LL * a[i] * b[j];
                int v = n - long(n * s) * mod;
                d += v >= mod ? v - mod : v < 0 ? v + mod : v;
                if (d >= mod)
                    d -= mod;
            }
        }
        for (int i = 0; i < N; i++) {
            auto n = 1LL * d * a[i];
            int v = n - long(n * s) * mod;
            d = v >= mod ? v - mod : v < 0 ? v + mod : v;
        }
        for (int j = 0; j < M; j++) {
            auto n = 1LL * d * b[j];
            int v = n - long(n * s) * mod;
            d = v >= mod ? v - mod : v < 0 ? v + mod : v;
        }
        assert(ans == d);
    }
}

int main() {
    RUN_BLOCK(stress_test_floor_sum());
    RUN_BLOCK(unit_test_modsqrt());
    RUN_BLOCK(unit_test_modlog());
    RUN_BLOCK(stress_test_modnum());
    return 0;
}
