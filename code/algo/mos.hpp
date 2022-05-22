#pragma once

#include "algo/y_combinator.hpp"

// Sort queries {l,r} for mo decomposition in the universe [0,N) with block size B
auto mosort_left_block(int N, const vector<array<int, 2>>& queries, int B = 0) {
    int Q = queries.size();
    if (B == 0 && Q > 0) {
        B = max<int>(1, N / sqrt(Q));
    }
    vector<pair<int, int>> block(Q);
    for (int i = 0; i < Q; i++) {
        auto [x, y] = queries[i];
        assert(0 <= x && x <= y && y <= N);
        block[i].first = x / B;
        block[i].second = (block[i].first & 1) ? -y : y;
    } // O(Q) divisions instead of O(Q log Q)
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
auto mosort_hilbert_curve(int N, const vector<array<int, 2>>& queries) {
    int K = N > 1 ? 8 * sizeof(int) - __builtin_clz(N - 1) : 0;
    int Q = queries.size();
    vector<long> ord(Q);
    for (int i = 0; i < Q; i++) {
        ord[i] = xy2d_hilbert(1 << K, queries[i][0], queries[i][1]);
    }
    vector<int> order(Q);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order), [&](int i, int j) { return ord[i] < ord[j]; });
    return order;
}

auto offline_range_distinct(const vector<int>& a, const vector<array<int, 2>>& queries) {
    int N = a.size(), Q = queries.size();
    auto order = mosort_left_block(N, queries);
    vector<int> ans(Q), pre(N), nxt(N);
    map<int, int> vals;
    for (int i = 0; i < N; i++) {
        int j = vals.count(a[i]) ? vals.at(a[i]) : -1;
        pre[i] = j, vals[a[i]] = i;
    }
    vals.clear();
    for (int i = N - 1; i >= 0; i--) {
        int j = vals.count(a[i]) ? vals.at(a[i]) : N;
        nxt[i] = j, vals[a[i]] = i;
    }
    vals.clear();
    int L = 0, R = 0, cnt = 0;
    for (int q : order) {
        auto [l, r] = queries[q];
        while (L > l) {
            cnt += nxt[--L] >= R;
        }
        while (R < r) {
            cnt += pre[R++] < L;
        }
        while (L < l) {
            cnt -= nxt[L++] >= R;
        }
        while (R > r) {
            cnt -= pre[--R] < L;
        }
        ans[q] = cnt;
    }
    return ans;
}
