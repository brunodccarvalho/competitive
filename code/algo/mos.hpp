#pragma once

#include "algo/y_combinator.hpp"

// Sort queries {l,r} for mo decomposition in the universe [0,N) with block size B
auto mosort_left_block(int N, const vector<pair<int, int>>& queries, int B = 0) {
    int Q = queries.size();
    if (B == 0 && Q > 0) {
        B = N / sqrt(Q);
    }
    vector<pair<int, int>> block(Q);
    for (int i = 0; i < Q; i++) {
        auto [x, y] = queries[i];
        block[i].first = x / B;
        block[i].second = (block[i].first & 1) ? -y : y;
    }
    vector<int> order(Q);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order), [&](int i, int j) { return block[i] < block[j]; });
    return order;
}

// Index of (x,y) along hilbert curve of size nxn power of two
int64_t xy2d_hilbert(int n, int x, int y) {
    int64_t d = 0;
    for (int64_t s = n / 2; s > 0; s >>= 1) {
        int rx = (x & s) > 0;
        int ry = (y & s) > 0;
        d += s * s * ((3 * ry) ^ rx);
        if (rx == 0) {
            if (ry == 1) {
                x = s - 1 - x;
                y = s - 1 - y;
            }
            swap(x, y);
        }
    }
    return d;
}

// Sort queries {l,r} for mo decomposition in the universe [0,N) with hilbert curve order
auto mosort_hilbert_curve(int N, const vector<pair<int, int>>& queries) {
    int K = N > 1 ? 8 * sizeof(int) - __builtin_clz(N - 1) : 0;
    int Q = queries.size();
    vector<long> ord(Q);
    for (int i = 0; i < Q; i++) {
        ord[i] = xy2d_hilbert(1 << K, queries[i].first, queries[i].second);
    }
    vector<int> order(Q);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order), [&](int i, int j) { return ord[i] < ord[j]; });
    return order;
}
