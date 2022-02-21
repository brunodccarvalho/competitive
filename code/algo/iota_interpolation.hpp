#pragma once

#include "numeric/combinatorics.hpp"

// Given polynomial P such that P[0,...,n)=y[0,...,n), compute P(x). O(n)
template <typename T>
auto interpolate_iota(const vector<T>& y, int64_t x) {
    int N = y.size();
    if (0 <= x && x < N) {
        return y[x];
    } else if (N == 0) {
        return T(0);
    }

    using B = Binomial<T>;
    B::ensure_factorial(N);

    vector<T> dp(N, 1), pd(N, 1);
    for (int i = 0; i < N - 1; i++) {
        dp[i + 1] = dp[i] * (x - i);
    }
    for (int i = N - 1; i > 0; i--) {
        pd[i - 1] = pd[i] * (x - i);
    }

    T val = 0;
    for (int i = 0; i < N; i++) {
        T cell = y[i] * dp[i] * pd[i] * B::ifact[i] * B::ifact[N - i - 1];
        val += ((N - i) & 1) ? cell : -cell;
    }
    return val;
}
