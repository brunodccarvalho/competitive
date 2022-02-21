#include "test_utils.hpp"
#include "numeric/convolution.hpp"
#include "numeric/modnum.hpp"

template <typename T>
auto naive_sos_subsets(const vector<T>& f) {
    int N = f.size();
    vector<T> F(N);
    for (int u = 0; u < N; u++) {
        for (int v = u; v; v = u & (v - 1)) {
            F[u] += f[v];
        }
        F[u] += f[0];
    }
    return F;
}

template <typename T>
auto naive_sos_supersets(const vector<T>& f) {
    int N = f.size();
    vector<T> F(N);
    for (int u = 0; u < N; u++) {
        for (int v = u; v; v = u & (v - 1)) {
            F[v] += f[u];
        }
        F[0] += f[u];
    }
    return F;
}

template <typename T>
auto naive_mobius_subsets(const vector<T>& f) {
    int N = f.size();
    vector<T> F(N);
    for (int mask = 0; mask < N; mask++) {
        for (int submask = 0; submask < N; submask++) {
            if ((submask & mask) == submask && (submask | mask) == mask) {
                int m = mask ? __builtin_popcount(mask) : 0;
                int s = submask ? __builtin_popcount(submask) : 0;
                int i = ((m - s) & 1) ? -1 : 1;
                F[mask] += i * f[submask];
            }
        }
    }
    return F;
}

template <typename T>
auto naive_mobius_supersets(const vector<T>& f) {
    int N = f.size();
    vector<T> F(N);
    for (int mask = 0; mask < N; mask++) {
        for (int submask = 0; submask < N; submask++) {
            if ((submask & mask) == submask && (submask | mask) == mask) {
                int m = mask ? __builtin_popcount(mask) : 0;
                int s = submask ? __builtin_popcount(submask) : 0;
                int i = ((m - s) & 1) ? -1 : 1;
                F[submask] += i * f[mask];
            }
        }
    }
    return F;
}

template <typename T>
auto naive_subset_convolution(const vector<T>& f, const vector<T>& g) {
    int N = f.size();
    vector<T> S(N);
    for (int mask = 0; mask < N; mask++) {
        S[mask] += f[0] * g[mask];
        for (int submask = mask; submask; submask = mask & (submask - 1)) {
            S[mask] += f[submask] * g[mask ^ submask];
        }
    }
    return S;
}

template <char type, typename T>
auto naive_convolution(const vector<T>& f, const vector<T>& g) {
    int N = f.size();
    vector<T> S(N);
    for (int u = 0; u < N; u++) {
        for (int v = 0; v < N; v++) {
            if constexpr (type == '^') {
                S[u ^ v] += f[u] * g[v];
            } else if constexpr (type == '&') {
                S[u & v] += f[u] * g[v];
            } else if constexpr (type == '|') {
                S[u | v] += f[u] * g[v];
            }
        }
    }
    return S;
}

template <typename T>
auto naive_xor_convolution(const vector<T>& f, const vector<T>& g) {
    return naive_convolution<'^', T>(f, g);
}

template <typename T>
auto naive_and_convolution(const vector<T>& f, const vector<T>& g) {
    return naive_convolution<'&', T>(f, g);
}

template <typename T>
auto naive_or_convolution(const vector<T>& f, const vector<T>& g) {
    return naive_convolution<'|', T>(f, g);
}

template <typename T> // O(n 3^n)
auto naive_exp(const vector<T>& f) {
    int N = f.size(), n = 8 * sizeof(N) - __builtin_clz(N - 1);
    vector<T> E(N);
    E[0] = 1;
    for (int i = 1; i <= n; i++) {
        for (int x = 0; x < N; x++) {
            if (__builtin_popcount(x) == i) {
                for (int s = x; s; s = x & (s - 1)) {
                    if (s > (x ^ s)) {
                        E[x] += E[x ^ s] * f[s];
                    }
                }
            }
        }
    }
    return E;
}

void stress_test_convolution() {
    using num = modnum<998244353>;

    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 10000, runs) {
        print_time(now, 20s, "stress convolution ({} runs)", runs);

        int n = 1 << rand_unif<int>(0, 13);
        vector<num> a = rands_unif<int, num>(n, 0, 5000);
        vector<num> b = rands_unif<int, num>(n, 0, 5000);
        b[0] = 1;

        assert(naive_xor_convolution(a, b) == xor_convolution(a, b));
        assert(naive_and_convolution(a, b) == and_convolution(a, b));
        assert(naive_or_convolution(a, b) == or_convolution(a, b));
        assert(naive_subset_convolution(a, b) == subset_convolution(a, b));
        assert(a == inverse_subset_convolution(subset_convolution(a, b), b));

        assert(naive_exp(b) == exp_conv(b));
        assert(exp_conv(log_conv(b)) == b);
        assert(log_conv(exp_conv(b)) == b);

        assert(naive_sos_subsets(a) == sos_subsets(a));
        assert(naive_sos_supersets(a) == sos_supersets(a));
        assert(naive_mobius_subsets(a) == mobius_subsets(a));
        assert(naive_mobius_supersets(a) == mobius_supersets(a));

        assert(a == naive_sos_subsets(naive_mobius_subsets(a)));
        assert(a == naive_sos_supersets(naive_mobius_supersets(a)));
        assert(a == sos_subsets(mobius_subsets(a)));
        assert(a == sos_supersets(mobius_supersets(a)));

        assert(a == naive_mobius_subsets(naive_sos_subsets(a)));
        assert(a == naive_mobius_supersets(naive_sos_supersets(a)));
        assert(a == mobius_subsets(sos_subsets(a)));
        assert(a == mobius_supersets(sos_supersets(a)));
    }
}

void speed_test_convolution() {
    using num = modnum<998244353>;
    vector<int> ns = {12, 13, 14, 15, 16, 17, 18, 19, 20};

    vector<int> inputs;
    for (int n : ns) {
        inputs.push_back(1 << n);
    }

    const auto runtime = 360'000ms / inputs.size();
    map<pair<string, int>, stringable> table;

    for (int N : inputs) {
        START_ACC5(xor, and, or, sos_subsets, sos_supersets);
        START_ACC4(subset, inverse, exp, log);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 1000, runs) {
            print_time(now, runtime, "speed convolution N={} ({} runs)", N, runs);

            auto a = rands_unif<int, num>(N, 0, 500'000'000);
            auto b = rands_unif<int, num>(N, 0, 500'000'000);

            ADD_TIME_BLOCK(xor) { xor_convolution(a, b); }
            ADD_TIME_BLOCK(and) { and_convolution(a, b); }
            ADD_TIME_BLOCK(or) { or_convolution(a, b); }
            ADD_TIME_BLOCK(sos_subsets) { sos_subsets(a); }
            ADD_TIME_BLOCK(sos_supersets) { sos_subsets(a); }

            b[0] = 1;

            ADD_TIME_BLOCK(subset) { subset_convolution(a, b); }
            ADD_TIME_BLOCK(inverse) { inverse_subset_convolution(a, b); }
            ADD_TIME_BLOCK(exp) { exp_conv(b); }
            ADD_TIME_BLOCK(log) { log_conv(b); }
        }

        table[{"xor", N}] = FORMAT_EACH(xor, runs);
        table[{"and", N}] = FORMAT_EACH(and, runs);
        table[{"or", N}] = FORMAT_EACH(or, runs);
        table[{"sos subsets", N}] = FORMAT_EACH(sos_subsets, runs);
        table[{"sos supersets", N}] = FORMAT_EACH(sos_supersets, runs);
        table[{"subset", N}] = FORMAT_EACH(subset, runs);
        table[{"inverse", N}] = FORMAT_EACH(inverse, runs);
        table[{"exp", N}] = FORMAT_EACH(exp, runs);
        table[{"log", N}] = FORMAT_EACH(log, runs);
    }

    print_time_table(table, "Convolution");
}

int main() {
    RUN_BLOCK(stress_test_convolution());
    RUN_BLOCK(speed_test_convolution());
    return 0;
}
