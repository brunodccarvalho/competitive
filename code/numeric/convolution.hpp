#pragma once

#include <bits/stdc++.h>
using namespace std;

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
        or_conv_transform<0>(fhat[i]);
        or_conv_transform<0>(ghat[i]);
    }

    vector<T> h;
    for (int i = 0; i <= n; i++) {
        h.assign(N, 0);
        for (int j = 0; j <= i; j++) {
            for (int x = 0; x < N; x++) {
                h[x] += fhat[j][x] * ghat[i - j][x];
            }
        }
        or_conv_transform<1>(h);
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
        or_conv_transform<0>(ghat[i]);
    }

    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= i; j++) {
            for (int x = 0; x < N; x++) {
                fhat[i][x] += ghat[j][x] * fhat[i - j][x];
            }
        }
        or_conv_transform<1>(fhat[i]);
        for (int x = 0; x < N; x++) {
            if (__builtin_popcount(x) == i) {
                f[x] = fhat[i][x] = (S[x] - fhat[i][x]) * inv;
            } else {
                fhat[i][x] = 0;
            }
        }
        or_conv_transform<0>(fhat[i]);
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
