#pragma once

#include "numeric/polynomial.hpp"

// Compute the smallest linear recurrence given in rec[0..n) by
//     x[n+k] = rec[0]x[n+k-1] + rec[1]x[n+k-2] + ... + rec[n-1]x[k], for all k>=0
// that produces the given output x. The output should be larger than 2n. O(n|x|)
template <typename T>
vector<T> berlekamp_massey(const vector<T>& x) {
    const int M = x.size();
    vector<T> b, c;
    if (M == 0) {
        return c;
    }

    b.reserve(M + 1), c.reserve(M + 1);
    b.push_back(-1), c.push_back(-1);
    T lead = 1;

    for (int i = 0; i < M; i++) {
        int N = c.size(), m = b.size();
        T delta = 0;
        for (int j = 0; j < N; j++) {
            delta -= c[j] * x[i + j - N + 1];
        }
        b.push_back(0), m++;
        if (delta == T()) {
            continue;
        }
        T fix = delta / lead;
        if (N < m) {
            auto a = c;
            c.insert(begin(c), m - N, T(0));
            for (int j = 0; j < m; j++) {
                c[m - 1 - j] -= fix * b[m - 1 - j];
            }
            b = move(a);
            lead = delta;
        } else {
            for (int j = 0; j < m; j++) {
                c[N - 1 - j] -= fix * b[m - 1 - j];
            }
        }
    }

    c.pop_back(), reverse(begin(c), end(c));
    return c;
}
