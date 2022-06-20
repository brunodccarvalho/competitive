#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T, typename BinOp>
struct bitree {
    int N;
    vector<T> tree;
    BinOp binop;

    explicit bitree(int N, T id = T()) : N(N), tree(N + 1, id), binop(BinOp()) {}

    explicit bitree(int N, const BinOp& op, T id = T())
        : N(N), tree(N + 1, id), binop(op) {}

    template <typename A>
    explicit bitree(int N, const vector<A>& arr, const BinOp& op = BinOp(), T id = T())
        : bitree(N, op, id) {
        for (int i = 1; i <= N; i++) {
            tree[i] = binop(arr[i - 1], tree[i]);
            if (int j = i + (i & -i); j <= N) {
                tree[j] = binop(tree[i], tree[j]);
            }
        }
    }

    void combine(int i, T v) {
        for (++i; i <= N; i += i & -i) {
            tree[i] = binop(tree[i], v);
        }
    }

    // Prefix sum exclusive, [0..r)
    T prefix(int r) const {
        T accum = tree[0];
        for (int i = r; i > 0; i -= i & -i) {
            accum = binop(tree[i], accum);
        }
        return accum;
    }

    // Find smallest i such that fn(prefix(i)) is true, or N otherwise.
    // fn should be F F F F ... T T T T, e.g. fn(sum) := x <= sum if sum is increasing
    template <typename Fn>
    int lower_bound(Fn&& fn) const {
        int i = 0;
        T accum = tree[0];
        for (int bit = 8 * sizeof(int) - __builtin_clz(N << 1); bit >= 0; bit--) {
            int next = i + (1 << bit);
            if (next <= N) {
                T combined = binop(accum, tree[next]);
                if (!fn(combined)) {
                    i = next;
                    accum = combined;
                }
            }
        }
        return i;
    }
};

template <typename T, typename BinOp>
struct bitree2d {
    int N, M;
    vector<vector<T>> tree;
    BinOp binop;

    explicit bitree2d(int N, int M, const BinOp& op, T id = T())
        : N(N), M(M), tree(N + 1, vector<T>(M + 1, id)), binop(op) {}

    template <typename A>
    explicit bitree2d(int N, int M, const vector<vector<A>>& arr, const BinOp& op,
                      T id = T())
        : bitree2d(N, M, op, id) {
        for (int i = 1; i <= N; i++) {
            for (int j = 1; j <= M; j++) {
                tree[i][j] = binop(arr[i - 1][j - 1], tree[i][j]);
            }
            if (int k = i + (i & -i); k <= N) {
                for (int j = 1; j <= M; j++) {
                    tree[k][j] = binop(tree[i][j], tree[k][j]);
                }
            }
        }
        for (int j = 1; j <= M; j++) {
            if (int k = j + (j & -j); k <= M) {
                for (int i = 1; i <= N; i++) {
                    tree[i][k] = binop(tree[i][j], tree[i][k]);
                }
            }
        }
    }

    void combine(int r, int c, T v) {
        for (int i = r + 1; i <= N; i += i & -i) {
            for (int j = c + 1; j <= N; j += j & -j) {
                tree[i][j] = binop(tree[i][j], v);
            }
        }
    }

    // Prefix sum exclusive, [0..r)[0..c)
    T prefix(int r, int c) const {
        T accum = tree[0][0];
        for (int i = r; i > 0; i -= i & -i) {
            for (int j = c; j > 0; j -= j & -j) {
                accum = binop(tree[i][j], accum);
            }
        }
        return accum;
    }
};
