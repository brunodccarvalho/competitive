#pragma once

#include "random.hpp"
#include "numeric/polynomial.hpp"

template <typename T> // Lead and last terms guaranteed to be nonzero
auto rand_recurrence(int n) {
    auto poly = polymath::rand_poly<T>(n);
    for (int i : {0, n - 1}) {
        if (poly[i] == T()) {
            poly[i] = T(1);
        }
    }
    return poly;
}

template <typename T>
void extend_recurrence_inplace(int M, vector<T>& x, const vector<T>& rec) {
    x.resize(M);
    int N = rec.size();
    for (int i = N; i < M; i++) {
        x[i] = 0;
        for (int j = 0; j < N; j++) {
            x[i] += rec[j] * x[i - j - 1];
        }
    }
}

template <typename T>
bool verify_recurrence(const vector<T>& x, const vector<T>& rec) {
    int M = x.size(), N = rec.size();
    for (int i = N; i < M; i++) {
        T want = 0;
        for (int j = 0; j < N; j++) {
            want += rec[j] * x[i - j - 1];
        }
        if (want != x[i]) {
            return false;
        }
    }
    return true;
}
