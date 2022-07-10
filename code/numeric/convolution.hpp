#pragma once

#include "algo/y_combinator.hpp"

// Generalized FFT-like fast convolutions in O(N log N)
template <char type, bool inverse, typename T>
void conv_transform(vector<T>& a) {
    int N = a.size();
    assert((N & (N - 1)) == 0);
    for (int h = 1; h < N; h *= 2) {
        for (int i = 0; i < N; i += 2 * h) {
            for (int j = i; j < i + h; j++) {
                if constexpr (type == '^') {
                    auto x = a[j];
                    auto y = a[j + h];
                    a[j] = x + y;
                    a[j + h] = x - y;
                } else if constexpr (type == '&' && !inverse) {
                    a[j] += a[j + h];
                } else if constexpr (type == '&' && inverse) {
                    a[j] -= a[j + h];
                } else if constexpr (type == '|' && !inverse) {
                    a[j + h] += a[j];
                } else if constexpr (type == '|' && inverse) {
                    a[j + h] -= a[j];
                }
            }
        }
    }
    if constexpr (inverse && type == '^') {
        for (int i = 0; i < N; i++) {
            a[i] /= T(N);
        }
    }
}

template <char type, typename T>
auto convolve(vector<T> a, vector<T> b) {
    assert(a.size() == b.size() && !a.empty());
    int N = a.size();
    assert((N & (N - 1)) == 0);
    conv_transform<type, 0>(a);
    conv_transform<type, 0>(b);
    for (int i = 0; i < N; i++) {
        a[i] = a[i] * b[i];
    }
    conv_transform<type, 1>(a);
    return a;
}

/**
 * Given f : [0..2^n) -> T, compute sum over subsets (aka SOS DP)
 * F[i]: SUM f[j], j∈i
 * Complexity: O(N log N) == O(n 2^n)
 */
template <typename T>
auto sos_subsets(const vector<T>& f) {
    int N = f.size();
    assert((N & (N - 1)) == 0);
    vector<T> F(begin(f), end(f));
    for (int bit = 1; bit < N; bit <<= 1) {
        for (int mask = 0; mask < N; mask++) {
            if (mask & bit) {
                F[mask] += F[mask ^ bit];
            }
        }
    }
    return F;
}

/**
 * Given f : [0..2^n) -> T, compute sum over supersets (aka SOS DP)
 * F[i]: SUM f[j], i∈j
 * Complexity: O(N log N) == O(n 2^n)
 */
template <typename T>
auto sos_supersets(const vector<T>& f) {
    int N = f.size();
    assert((N & (N - 1)) == 0);
    vector<T> F(begin(f), end(f));
    for (int bit = 1; bit < N; bit <<= 1) {
        for (int mask = 0; mask < N; mask++) {
            if (mask & bit) {
                F[mask ^ bit] += F[mask];
            }
        }
    }
    return F;
}

/**
 * Given f : [0..2^n) -> T, compute mobius sum over subsets (inverse of SOS)
 * F[i]: SUM (-1)^|i-j| f[j], j∈i
 * Complexity: O(N log N) == O(n 2^n)
 */
template <typename T>
auto mobius_subsets(const vector<T>& f) {
    int N = f.size();
    assert((N & (N - 1)) == 0);
    vector<T> F(begin(f), end(f));
    for (int bit = 1; bit < N; bit <<= 1) {
        for (int mask = 0; mask < N; mask++) {
            if (mask & bit) {
                F[mask] -= F[mask ^ bit];
            }
        }
    }
    return F;
}

/**
 * Given f : [0..2^n) -> T, compute mobius sum over supersets (inverse of SOS)
 * F[i]: SUM (-1)^|i-j| f[j], i∈j
 * Complexity: O(N log N) == O(n 2^n)
 */
template <typename T>
auto mobius_supersets(const vector<T>& f) {
    int N = f.size();
    assert((N & (N - 1)) == 0);
    vector<T> F(begin(f), end(f));
    for (int bit = 1; bit < N; bit <<= 1) {
        for (int mask = 0; mask < N; mask++) {
            if (mask & bit) {
                F[mask ^ bit] -= F[mask];
            }
        }
    }
    return F;
}

/**
 * Given f : [0..2^n) -> T and g : [0..2^n) -> T, compute subset sum convolution
 * S[i]: SUM f[a]g[b], a&b=0, a|b=i
 * Complexity: O(N log^2 N) == O(n^2 2^n)
 * Reference: https://codeforces.com/blog/entry/72488
 */
template <typename T>
void subset_convolution(int n, const T* f, const T* g, T* S) {
    int N = 1 << n;
    vector<vector<T>> fhat(n + 1, vector<T>(N));
    vector<vector<T>> ghat(n + 1, vector<T>(N));

    for (int mask = 0; mask < N; mask++) {
        fhat[__builtin_popcount(mask)][mask] = f[mask];
        ghat[__builtin_popcount(mask)][mask] = g[mask];
    }
    for (int i = 0; i <= n; i++) {
        conv_transform<'|', 0>(fhat[i]);
        conv_transform<'|', 0>(ghat[i]);
    }

    vector<T> h;
    for (int i = 0; i <= n; i++) {
        h.assign(N, 0);
        for (int j = 0; j <= i; j++) {
            for (int x = 0; x < N; x++) {
                h[x] += fhat[j][x] * ghat[i - j][x];
            }
        }
        conv_transform<'|', 1>(h);
        for (int x = 0; x < N; x++) {
            if (__builtin_popcount(x) == i) {
                S[x] = h[x];
            }
        }
    }
}

template <typename T>
auto subset_convolution(const vector<T>& f, const vector<T>& g) {
    int N = f.size(), n = N > 1 ? 8 * sizeof(N) - __builtin_clz(N - 1) : 0;
    assert(N == (1 << n) && N == int(g.size()));
    vector<T> S(N);
    subset_convolution(n, f.data(), g.data(), S.data());
    return S;
}

/**
 * Given S : [0..2^n) -> T and g : [0..2^n) -> T, compute f such that f*g = S
 * S[i]: SUM f[a]g[b], a&b=0, a|b=i
 * Complexity: O(N log^2 N) == O(n^2 2^n)
 * Reference: https://gist.github.com/dario2994/e3257326ee80c054d3b48766b600991a
 * g[0] must be invertible.
 */
template <typename T>
void inverse_subset_convolution(int n, const T* S, const T* g, T* f) {
    int N = 1 << n;
    T inv = T(1) / g[0];

    vector<vector<T>> fhat(n + 1, vector<T>(N));
    vector<vector<T>> ghat(n + 1, vector<T>(N));

    for (int mask = 0; mask < N; mask++) {
        ghat[__builtin_popcount(mask)][mask] = g[mask];
    }
    for (int i = 0; i <= n; i++) {
        conv_transform<'|', 0>(ghat[i]);
    }

    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= i; j++) {
            for (int x = 0; x < N; x++) {
                fhat[i][x] += ghat[j][x] * fhat[i - j][x];
            }
        }
        conv_transform<'|', 1>(fhat[i]);
        for (int x = 0; x < N; x++) {
            if (__builtin_popcount(x) == i) {
                f[x] = fhat[i][x] = (S[x] - fhat[i][x]) * inv;
            } else {
                fhat[i][x] = 0;
            }
        }
        conv_transform<'|', 0>(fhat[i]);
    }
}

template <typename T>
auto inverse_subset_convolution(const vector<T>& S, const vector<T>& g) {
    int N = S.size(), n = N > 1 ? 8 * sizeof(N) - __builtin_clz(N - 1) : 0;
    assert(N == (1 << n) && N == int(g.size()));
    vector<T> f(N);
    inverse_subset_convolution(n, S.data(), g.data(), f.data());
    return f;
}

/**
 * Given f : [0..2^n) -> T, compute exp(f) as
 * E[i]: SUM f[x1]f[x2]...f[xk], xi&xj=0, x1|...|xk=i
 * Complexity: O(N log^2 N) = O(n^2 2^n)
 */
template <typename T>
auto exp_conv(const vector<T>& f) {
    int N = f.size(), n = 8 * sizeof(N) - __builtin_clz(N - 1);
    assert((N & (N - 1)) == 0);

    vector<T> E(N);
    E[0] = 1;
    for (int i = 0; i < n; i++) {
        subset_convolution(i, f.data() + (1 << i), E.data(), E.data() + (1 << i));
    }
    return E;
}

/**
 * Given f : [0..2^n) -> T, compute log(f) such that exp(log(f)) = f
 * Complexity: O(N log^2 N) = O(n^2 2^n)
 */
template <typename T>
auto log_conv(const vector<T>& f) {
    int N = f.size(), n = 8 * sizeof(N) - __builtin_clz(N - 1);
    assert((N & (N - 1)) == 0 && f[0] == 1);
    vector<T> E(N);
    E[0] = 1;
    for (int i = 0; i < n; i++) {
        inverse_subset_convolution(i, f.data() + (1 << i), f.data(), E.data() + (1 << i));
    }
    return E;
}

template <bool inverse, typename T>
void lcm_transform(vector<T>& a, const vector<int>& primes) {
    int n = a.size() - 1;
    for (int i = 0, P = primes.size(); i < P && primes[i] <= n; i++) {
        if constexpr (inverse) {
            for (int p = primes[i], k = n / p; k > 0; k--) {
                a[k * p] -= a[k];
            }
        } else {
            for (int p = primes[i], k = 1; k <= n / p; k++) {
                a[k * p] += a[k];
            }
        }
    }
}

template <bool inverse, typename T>
void gcd_transform(vector<T>& a, const vector<int>& primes) {
    int n = a.size() - 1;
    for (int i = 0, P = primes.size(); i < P && primes[i] <= n; i++) {
        if constexpr (inverse) {
            for (int p = primes[i], k = 1; k <= n / p; k++) {
                a[k] -= a[k * p];
            }
        } else {
            for (int p = primes[i], k = n / p; k > 0; k--) {
                a[k] += a[k * p];
            }
        }
    }
}

/**
 * Given f : [0..n] -> T and g : [0..n] -> T, compute gcd convolution
 * S[i]: SUM f[a]g[b], gcd(a,b)=i, S[0]=f[0]g[0]
 * Complexity: O(n log n), https://judge.yosupo.jp/problem/gcd_convolution
 */
template <typename T>
auto gcd_convolution(vector<T> a, vector<T> b, const vector<int>& primes) {
    int N = max(a.size(), b.size());
    a.resize(N, 0), b.resize(N, 0);
    gcd_transform<0>(a, primes);
    gcd_transform<0>(b, primes);
    for (int i = 0; i < N; i++) {
        a[i] = a[i] * b[i];
    }
    gcd_transform<1>(a, primes);
    return a;
}

/**
 * Given f : [0..n] -> T and g : [0..n] -> T, compute lcm convolution
 * S[i]: SUM f[a]g[b], lcm(a,b)=i, S[0]=f[0]g[0]
 * Complexity: O(n log n), https://judge.yosupo.jp/problem/lcm_convolution
 */
template <typename T>
auto lcm_convolution(vector<T> a, vector<T> b, const vector<int>& primes) {
    int N = max(a.size(), b.size());
    a.resize(N, 0), b.resize(N, 0);
    lcm_transform<0>(a, primes);
    lcm_transform<0>(b, primes);
    for (int i = 0; i < N; i++) {
        a[i] = a[i] * b[i];
    }
    lcm_transform<1>(a, primes);
    return a;
}

// Compute min plus convolution c[k] = min{i+j=k}(a[i]+b[j]), for convex a & b. O(N + M)
template <typename V>
auto min_plus_convex_minkowski(const vector<V>& a, const vector<V>& b) {
    int N = a.size(), M = b.size();
    if (N == 0 || M == 0) {
        return N ? a : b;
    }
    vector<V> c(N + M - 1);
    int i = 0, j = 0;
    while (i < N - 1 || j < N - 1) {
        if (j == M - 1) {
            c[i + j] = a[i++] + b[j];
        } else if (i == N - 1) {
            c[i + j] = a[i] + b[j++];
        } else if (a[i + 1] - a[i] <= b[j + 1] - b[j]) {
            c[i + j] = a[i++] + b[j];
        } else {
            c[i + j] = a[i] + b[j++];
        }
    }
    return c;
}

// Compute max plus convolution c[k] = max{i+j=k}(a[i]+b[j]), for concave a & b. O(N + M)
template <typename V>
auto max_plus_concave_minkowski(const vector<V>& a, const vector<V>& b) {
    int N = a.size(), M = b.size();
    if (N == 0 || M == 0) {
        return N ? a : b;
    }
    vector<V> c(N + M - 1);
    int i = 0, j = 0;
    while (i < N - 1 || j < N - 1) {
        if (j == M - 1) {
            c[i + j] = a[i++] + b[j];
        } else if (i == N - 1) {
            c[i + j] = a[i] + b[j++];
        } else if (a[i + 1] - a[i] >= b[j + 1] - b[j]) {
            c[i + j] = a[i++] + b[j];
        } else {
            c[i + j] = a[i] + b[j++];
        }
    }
    return c;
}

// Compute min plus convolution c[k] = min{i+j=k}(a[i]+b[j]), for concave a & b. O(N + M)
template <typename V>
auto min_plus_concave_border(const vector<V>& a, const vector<V>& b) {
    int N = a.size(), M = b.size();
    if (N == 0 || M == 0) {
        return N ? a : b;
    }
    vector<V> c(N + M - 1);
    for (int k = 0; k < N + M - 1; k++) {
        int li = max(0, k - M + 1), ri = min(k, N - 1);
        int lj = max(0, k - N + 1), rj = min(k, M - 1);
        c[k] = min(a[li] + b[rj], a[ri] + b[lj]);
    }
    return c;
}

// Compute max plus convolution c[k] = min{i+j=k}(a[i]+b[j]), for convex a & b. O(N + M)
template <typename V>
auto max_plus_convex_border(const vector<V>& a, const vector<V>& b) {
    int N = a.size(), M = b.size();
    if (N == 0 || M == 0) {
        return N ? a : b;
    }
    vector<V> c(N + M - 1);
    for (int k = 0; k < N + M - 1; k++) {
        int li = max(0, k - M + 1), ri = min(k, N - 1);
        int lj = max(0, k - N + 1), rj = min(k, M - 1);
        c[k] = max(a[li] + b[rj], a[ri] + b[lj]);
    }
    return c;
}

// Compute row-minima index for totally monotone f(x,y). Ties not broken. O(N + M)
template <typename Fn>
auto min_smawk(Fn&& f, int R, int C) {
    using vi = vector<int>;
    vi initrow(R), initcol(C);
    iota(begin(initrow), end(initrow), 0);
    iota(begin(initcol), end(initcol), 0);
    return y_combinator([&](auto self, const vi& row, vi col) -> vi {
        int N = row.size(), M = col.size();
        if (N == 1) {
            int ans = *min_element(begin(col), end(col), [&](int a, int b) {
                return make_pair(f(row[0], a), a) < make_pair(f(row[0], b), b);
            });
            return {ans};
        }
        if (N < M) {
            // Reduce number of columns to number of rows
            vector<array<int, 2>> minima;
            for (int c : col) {
                while (!minima.empty()) {
                    auto [r, p] = minima.back();
                    if (f(r, c) < f(r, p)) {
                        minima.pop_back();
                    } else if (r + 1 < N) {
                        minima.push_back({r + 1, c});
                        break;
                    } else {
                        break;
                    }
                }
                if (minima.empty()) {
                    minima.push_back({0, c});
                }
            }
            while (minima.back()[0] >= N) {
                minima.pop_back();
            }
            M = minima.size();
            for (int i = 0; i < M; i++) {
                col[i] = minima[i][1];
            }
        }
        vector<int> even(N / 2), ans(N);
        for (int i = 1; i < N; i += 2) {
            even[i >> 1] = row[i];
        }
        auto even_ans = self(even, col);
        for (int i = 0, it = 0; i < N; i++) {
            if (i % 2) {
                ans[i] = even_ans[i >> 1];
            } else {
                int last = i < N - 1 ? even_ans[i >> 1] : col.back();
                ans[i] = col[it];
                while (it < last) {
                    if (f(row[i], col[++it]) < f(row[i], ans[i])) {
                        ans[i] = col[it];
                    }
                }
            }
        }
        return ans;
    })(initrow, initcol);
}

// Compute row-maxima index for totally monotone f(x,y). Ties not broken. O(N + M)
template <typename Fn>
auto max_smawk(Fn&& f, int R, int C) {
    return min_smawk([&f](auto r, auto c) { return -f(r, c); }, R, C);
}

// Compute min plus convolution c[k] = min{i+j=k}(a[i]+b[j]) for convex b. O(N + M)
template <typename V>
auto min_plus_smawk(const vector<V>& a, const vector<V>& b) {
    static constexpr V inf = numeric_limits<V>::max() / 3;
    int N = a.size();
    int M = b.size();
    if (N == 0 || M == 0) {
        return N ? a : b;
    }
    auto f = [&](int r, int c) {
        if (r < c) {
            return inf + r + c;
        } else if (r - c >= M) {
            return inf - r - c;
        } else {
            return a[c] + b[r - c];
        }
    };
    auto cols = min_smawk(f, N + M - 1, N);
    vector<V> d(N + M - 1);
    for (int r = 0; r < N + M - 1; r++) {
        d[r] = f(r, cols[r]);
    }
    return d;
}

// Compute max plus convolution c[k] = min{i+j=k}(a[i]+b[j]) for concave b. O(N + M)
template <typename V>
auto max_plus_smawk(const vector<V>& a, const vector<V>& b) {
    static constexpr V inf = numeric_limits<V>::lowest() / 3;
    int N = a.size();
    int M = b.size();
    if (N == 0 || M == 0) {
        return N ? a : b;
    }
    auto f = [&](int r, int c) {
        if (r < c) {
            return inf - r - c;
        } else if (r - c >= M) {
            return inf + r + c;
        } else {
            return a[c] + b[r - c];
        }
    };
    auto cols = max_smawk(f, N + M - 1, N);
    vector<V> d(N + M - 1);
    for (int r = 0; r < N + M - 1; r++) {
        d[r] = f(r, cols[r]);
    }
    return d;
}
