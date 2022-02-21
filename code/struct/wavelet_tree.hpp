#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Basic wavelet tree on ints
 * O(N log Σ) memory, all operations are O(log Σ)
 */
struct wavelet_tree {
    int A, min_sigma, max_sigma; // [min_sigma, max_sigma)
    vector<vector<int>> data;

    wavelet_tree(int min_sigma, int max_sigma, const vector<int>& arr)
        : A(arr.size()), min_sigma(min_sigma), max_sigma(max_sigma),
          data((max_sigma - min_sigma) << 1) {
        vector<int> brr(arr);
        build_dfs(1, min_sigma, max_sigma, 0, A, brr);
    }

    // Count occurrences of x in arr[L,R)
    int count_equal(int L, int R, int x) const {
        return count_equal(R, x) - count_equal(L, x);
    }

    int count_equal(int R, int x) const {
        int u = 1, l = min_sigma, r = max_sigma;
        while (l + 1 < r) {
            int m = l + (r - l) / 2;
            if (x < m) { // go the left subtree
                R = data[u][R];
                r = m, u = u << 1;
            } else { // go to the right subtree
                R = R - data[u][R];
                l = m, u = u << 1 | 1;
            }
        }
        return R;
    }

    // Count occurrences of [x,y) in arr[L,R)
    int count_within(int L, int R, int x, int y) const {
        return count_dfs(1, min_sigma, max_sigma, x, y, L, R);
    }

    // Count elements less than x in arr[L,R)
    int order_of_key(int L, int R, int x) const {
        return count_within(L, R, min_sigma, x);
    }

    // Find k-th smallest element in [L,R)
    int find_by_order(int L, int R, int kth) const {
        int u = 1, l = min_sigma, r = max_sigma;
        while (l + 1 < r) {
            int m = l + (r - l) / 2;
            if (kth < data[u][R] - data[u][L]) { // lots of small values on the left
                L = data[u][L], R = data[u][R];
                r = m, u = u << 1;
            } else { // few small values on the left, skip them and search on the right
                kth -= data[u][R] - data[u][L];
                L = L - data[u][L], R = R - data[u][R];
                l = m, u = u << 1 | 1;
            }
        }
        return l;
    }

  private:
    void build_dfs(int u, int l, int r, int aL, int aR, vector<int>& arr) {
        if (l + 1 < r) {
            int m = l + (r - l) / 2;
            data[u].resize(aR - aL + 1);
            for (int i = aL; i < aR; i++) {
                data[u][i - aL + 1] = data[u][i - aL] + (arr[i] < m);
            }
            auto cmp = [&](int x) { return x < m; };
            int mi = stable_partition(begin(arr) + aL, begin(arr) + aR, cmp) - begin(arr);
            build_dfs(u << 1, l, m, aL, mi, arr);
            build_dfs(u << 1 | 1, m, r, mi, aR, arr);
        }
    }

    int count_dfs(int u, int l, int r, int x, int y, int aL, int aR) const {
        if ((x <= l && r <= y) || aR == aL) {
            return aR - aL;
        }
        int m = l + (r - l) / 2;
        if (y <= m) {
            return count_dfs(u << 1, l, m, x, y, data[u][aL], data[u][aR]);
        } else if (m <= x) {
            return count_dfs(u << 1 | 1, m, r, x, y, aL - data[u][aL], aR - data[u][aR]);
        } else {
            return count_dfs(u << 1, l, m, x, y, data[u][aL], data[u][aR]) +
                   count_dfs(u << 1 | 1, m, r, x, y, aL - data[u][aL], aR - data[u][aR]);
        }
    }
};
