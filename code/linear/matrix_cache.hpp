#pragma once

#include "numeric/modnum.hpp"

namespace std {

template <typename V, size_t M>
using Mat = array<array<V, M>, M>;
template <typename V, size_t M>
using Vec = array<V, M>;

template <typename V, size_t M>
auto identity() {
    Mat<V, M> a = {};
    for (unsigned i = 0; i < M; i++)
        a[i][i] = 1;
    return a;
}

template <typename V, size_t M>
auto operator+(const Mat<V, M>& a, const Mat<V, M>& b) {
    Mat<V, M> c = {};
    for (unsigned i = 0; i < M; i++)
        for (unsigned j = 0; j < M; j++)
            c[i][j] = a[i][j] + b[i][j];
    return c;
}

template <typename V, size_t M>
auto operator*(const Mat<V, M>& a, const Mat<V, M>& b) {
    Mat<V, M> c = {};
    for (unsigned i = 0; i < M; i++)
        for (unsigned k = 0; k < M; k++)
            for (unsigned j = 0; j < M; j++)
                c[i][j] += a[i][k] * b[k][j];
    return c;
}

template <typename V, size_t M>
auto operator*(const Mat<V, M>& a, const Vec<V, M>& b) {
    Vec<V, M> c = {};
    for (unsigned i = 0; i < M; i++)
        for (unsigned j = 0; j < M; j++)
            c[i] += a[i][j] * b[j];
    return c;
}

template <typename V, size_t M>
auto operator^(const Mat<V, M>& a, int64_t k) {
    Vec<V, M> b = a, c = identity<V, M>();
    while (k > 0) {
        if (k & 1)
            c = c * b;
        if (k >>= 1)
            b = b * b;
    }
    return c;
}

} // namespace std

// Matrix cache for fast matrix exponentiation
template <typename V, size_t M, int B, int S>
struct MatrixCache {
    Mat<V, M> A, jmp[B][S + 1];

    explicit MatrixCache(Mat<V, M> A) : A(A) { build(); }

    void build() {
        jmp[0][0] = identity<V, M>();
        for (int s = 1; s <= S; s++) {
            jmp[0][s] = A * jmp[0][s - 1];
        }
        for (int b = 1; b < B; b++) {
            jmp[b][0] = identity<V, M>();
            for (int s = 1; s <= S; s++) {
                jmp[b][s] = jmp[b - 1][S] * jmp[b][s - 1];
            }
        }
    }

    auto get(int64_t k) const {
        assert(k >= 0);
        auto m = jmp[0][k % S];
        k /= S;
        for (int b = 1; b < B && k > 0; b++, k /= S) {
            m = m * jmp[b][k % S];
        }
        return m;
    }
};

template <typename V>
struct Fibonacci {
    MatrixCache<V, 2, 4, 2 << 16> cache;

    Fibonacci() : cache({{{0, 1}, {1, 1}}}) {}

    // Fib(x)
    auto get(int64_t x) const { return cache.get(x + 1)[0][0]; }

    // {Fib(x),Fib(x+1)}
    auto get_pair(int64_t x) const { return cache.get(x + 1)[0]; }

    // Given a=SUM Fib(x) and b=SUM Fib(x+1), compute SUM Fib(x+c) and SUM Fib(x+1+c)
    auto shift(V a, V b, int64_t c) { return cache.get(c) * Vec<V, 2>({a, b}); }
};
