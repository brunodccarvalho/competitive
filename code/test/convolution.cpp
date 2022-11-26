#include "test_utils.hpp"
#include "numeric/convolution.hpp"
#include "numeric/modnum.hpp"
#include "numeric/sieves.hpp"

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

template <typename T>
auto naive_gcd_convolution(const vector<T>& a, const vector<T>& b) {
    int N = a.size(), M = b.size();
    vector<T> c(N);
    c[0] = a[0] * b[0];
    for (int i = 1; i < N; i++)
        for (int j = 1; j < M; j++)
            c[gcd(i, j)] += a[i] * b[j];
    return c;
}

template <typename T>
auto naive_lcm_convolution(const vector<T>& a, const vector<T>& b) {
    int N = a.size(), M = b.size();
    vector<T> c(N);
    c[0] = a[0] * b[0];
    for (int i = 1; i < N; i++)
        for (int j = 1; j < M; j++)
            if (auto l = lcm(i, j); l < N)
                c[lcm(i, j)] += a[i] * b[j];
    return c;
}

template <bool inverse, typename T>
void xor_conv_transform(vector<T>& a) {
    conv_transform<'^', inverse>(a);
}
template <bool inverse, typename T>
void and_conv_transform(vector<T>& a) {
    conv_transform<'&', inverse>(a);
}
template <bool inverse, typename T>
void or_conv_transform(vector<T>& a) {
    conv_transform<'|', inverse>(a);
}
template <typename T>
auto xor_convolution(const vector<T>& a, const vector<T>& b) {
    return convolve<'^'>(a, b);
}
template <typename T>
auto and_convolution(const vector<T>& a, const vector<T>& b) {
    return convolve<'&'>(a, b);
}
template <typename T>
auto or_convolution(const vector<T>& a, const vector<T>& b) {
    return convolve<'|'>(a, b);
}

template <typename T>
auto naive_min_plus(const vector<T>& a, const vector<T>& b) {
    int N = a.size(), M = b.size(), S = N + M - 1;
    vector<T> c(S, numeric_limits<T>::max());
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            c[i + j] = min(c[i + j], a[i] + b[j]);
        }
    }
    return c;
}

template <typename T>
auto naive_max_plus(const vector<T>& a, const vector<T>& b) {
    int N = a.size(), M = b.size(), S = N + M - 1;
    vector<T> c(S, numeric_limits<T>::lowest());
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            c[i + j] = max(c[i + j], a[i] + b[j]);
        }
    }
    return c;
}

auto random_convex(int n) {
    auto a = rands_unif<int64_t>(n, 0, 5'000'000);
    sort(begin(a), end(a));
    for (int i = 1; i < n; i++) {
        a[i] += a[i - 1];
    }
    for (int i = 1; i + 1 < n; i++) {
        assert(2 * a[i] <= a[i - 1] + a[i + 1]);
    }
    return a;
}

auto random_concave(int n) {
    auto a = rands_unif<int64_t>(n, 0, 5'000'000);
    sort(rbegin(a), rend(a));
    for (int i = 1; i < n; i++) {
        a[i] += a[i - 1];
    }
    for (int i = 1; i + 1 < n; i++) {
        assert(2 * a[i] >= a[i - 1] + a[i + 1]);
    }
    return a;
}

void stress_test_convolution() {
    using num = modnum<998244353>;
    auto primes = classic_sieve(50000);

    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 10000, runs) {
        print_time(now, 20s, "stress convolution ({} runs)", runs);

        int n = 1 << rand_unif<int>(0, 4);
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

        assert(naive_gcd_convolution(a, b) == gcd_convolution(a, b, primes));
        assert(naive_lcm_convolution(a, b) == lcm_convolution(a, b, primes));
    }
}

void speed_test_convolution() {
    using num = modnum<998244353>;
    vector<int> ns = {12, 13, 14, 15, 16, 17, 18, 19, 20};

    vector<int> inputs;
    for (int n : ns) {
        inputs.push_back(1 << n);
    }

    const auto runtime = 240'000ms / inputs.size();
    map<pair<string, int>, stringable> table;

    for (int N : inputs) {
        START_ACC5(xor, and, or, sos_subsets, sos_supersets);
        START_ACC4(subset, inverse, exp, log);
        START_ACC2(gcd, lcm);
        auto primes = classic_sieve(N);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 200, runs) {
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
            ADD_TIME_BLOCK(gcd) { gcd_convolution(a, b, primes); }
            ADD_TIME_BLOCK(lcm) { lcm_convolution(a, b, primes); }
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
        table[{"gcd", N}] = FORMAT_EACH(gcd, runs);
        table[{"lcm", N}] = FORMAT_EACH(lcm, runs);
    }

    print_time_table(table, "Convolution");
}

void stress_test_plus_convolution() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 10000, runs) {
        print_time(now, 20s, "stress min/max plus convolution ({} runs)", runs);

        int n = rand_unif<int>(1, 3000);
        int m = rand_unif<int>(1, 3000);
        auto a = random_convex(n);
        auto b = random_concave(n);
        auto c = random_convex(m);
        auto d = random_concave(m);
        auto e = rands_unif<int64_t>(n, -500'000'000, +500'000'000);

        assert(naive_min_plus(a, c) == min_plus_convex_minkowski(a, c));
        assert(naive_max_plus(a, c) == max_plus_convex_border(a, c));
        assert(naive_min_plus(a, d) == min_plus_concave_one(a, d));
        assert(naive_max_plus(a, d) == max_plus_smawk(a, d));
        assert(naive_min_plus(b, c) == min_plus_smawk(b, c));
        assert(naive_max_plus(b, c) == max_plus_convex_one(b, c));
        assert(naive_min_plus(b, d) == min_plus_concave_border(b, d));
        assert(naive_max_plus(b, d) == max_plus_concave_minkowski(b, d));

        assert(naive_min_plus(e, a) == min_plus_smawk(e, a));
        assert(naive_max_plus(e, b) == max_plus_smawk(e, b));
        assert(naive_min_plus(e, b) == min_plus_concave_one(e, b));
        assert(naive_min_plus(e, a) == min_plus_convex_one(e, a));
        assert(naive_max_plus(e, a) == max_plus_convex_one(e, a));
        assert(naive_max_plus(e, b) == max_plus_concave_one(e, b));
    }
}

void speed_test_plus_convolution() {
    vector<int> ns = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};

    vector<int> inputs = ns;

    const auto runtime = 120'000ms / inputs.size();
    map<pair<string, int>, stringable> table;

    for (int N : inputs) {
        START_ACC5(minkowski, border, smawk, smawk_a, smawk_b);
        START_ACC4(convex1d1d, convex1d1d_a, convex1d1d_b, concave1d1d);
        auto primes = classic_sieve(N);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 200, runs) {
            print_time(now, runtime, "speed convolution N={} ({} runs)", N, runs);

            int n = N - rand_unif<int>(0, 30);
            int m = N - rand_unif<int>(0, 30);
            int k = rand_unif<int>(90, 160);

            auto a = random_convex(n);
            auto b = random_concave(n);
            auto c = random_convex(m);
            auto d = random_concave(m);
            auto e = rands_unif<int64_t>(n, -500'000'000, +500'000'000);
            auto f = random_convex(k);
            auto g = rands_unif<int64_t>(k, -500'000'000, 500'000'000);

            ADD_TIME_BLOCK(minkowski) { min_plus_convex_minkowski(a, c); }
            ADD_TIME_BLOCK(border) { min_plus_concave_border(b, d); }
            ADD_TIME_BLOCK(smawk) { min_plus_smawk(e, a); }
            ADD_TIME_BLOCK(convex1d1d) { min_plus_convex_one(e, a); }
            ADD_TIME_BLOCK(concave1d1d) { min_plus_concave_one(e, b); }
            ADD_TIME_BLOCK(smawk_a) { min_plus_smawk(g, a); }
            ADD_TIME_BLOCK(smawk_b) { min_plus_smawk(e, f); }
            ADD_TIME_BLOCK(convex1d1d_a) { min_plus_convex_one(g, a); }
            ADD_TIME_BLOCK(convex1d1d_b) { min_plus_convex_one(e, f); }
        }

        table[{"minkowski", N}] = FORMAT_EACH(minkowski, runs);
        table[{"border", N}] = FORMAT_EACH(border, runs);
        table[{"smawk", N}] = FORMAT_EACH(smawk, runs);
        table[{"smawk-a", N}] = FORMAT_EACH(smawk_a, runs);
        table[{"smawk-b", N}] = FORMAT_EACH(smawk_b, runs);
        table[{"convex-1d1d", N}] = FORMAT_EACH(convex1d1d, runs);
        table[{"convex-1d1d-a", N}] = FORMAT_EACH(convex1d1d_a, runs);
        table[{"convex-1d1d-b", N}] = FORMAT_EACH(convex1d1d_b, runs);
        table[{"concave-1d1d", N}] = FORMAT_EACH(concave1d1d, runs);
    }

    print_time_table(table, "Plus Convolution");
}

int main() {
    RUN_BLOCK(stress_test_plus_convolution());
    RUN_BLOCK(speed_test_plus_convolution());
    RUN_BLOCK(stress_test_convolution());
    RUN_BLOCK(speed_test_convolution());
    return 0;
}
