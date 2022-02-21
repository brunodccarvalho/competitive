#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Source: https://github.com/Aeren1564/Algorithms/.../mergesort_tree.sublime-snippet
 */
template <typename T, typename Compare = less<T>>
struct mergesort_tree {
    int N;
    Compare cmp;
    vector<vector<T>> st;

    explicit mergesort_tree(const vector<T>& arr, Compare cmp = Compare())
        : N(arr.size()), cmp(cmp), st(N << 1) {
        for (int i = N; i < 2 * N; i++) {
            st[i].push_back(arr[i - N]);
        }
        for (int i = N - 1; i > 0; i--) {
            merge(begin(st[i << 1]), end(st[i << 1]), begin(st[i << 1 | 1]),
                  end(st[i << 1 | 1]), back_inserter(st[i]), cmp);
        }
    }

    // O(log^2 D)
    int count_less(int l, int r, const T& key) const {
        assert(0 <= l && l <= r && r <= N);
        int ans = 0;
        for (l += N, r += N; l < r; l >>= 1, r >>= 1) {
            if (l & 1) {
                ans += lower_bound(begin(st[l]), end(st[l]), key, cmp) - begin(st[l]);
                ++l;
            }
            if (r & 1) {
                --r;
                ans += lower_bound(begin(st[r]), end(st[r]), key, cmp) - begin(st[r]);
            }
        }
        return ans;
    }
    // O(log^2 D)
    int count_equal_or_less(int l, int r, const T& key) const {
        assert(0 <= l && l <= r && r <= N);
        int ans = 0;
        for (l += N, r += N; l < r; l >>= 1, r >>= 1) {
            if (l & 1) {
                ans += upper_bound(begin(st[l]), end(st[l]), key, cmp) - begin(st[l]);
                ++l;
            }
            if (r & 1) {
                --r;
                ans += upper_bound(begin(st[r]), end(st[r]), key, cmp) - begin(st[r]);
            }
        }
        return ans;
    }
    // O(log^2 D)
    int count_greater(int l, int r, const T& key) const {
        assert(0 <= l && l <= r && r <= N);
        int ans = 0;
        for (l += N, r += N; l < r; l >>= 1, r >>= 1) {
            if (l & 1) {
                ans += end(st[l]) - upper_bound(begin(st[l]), end(st[l]), key, cmp);
                ++l;
            }
            if (r & 1) {
                --r;
                ans += end(st[r]) - upper_bound(begin(st[r]), end(st[r]), key, cmp);
            }
        }
        return ans;
    }
    // O(log^2 D)
    int count_equal_or_greater(int l, int r, const T& key) const {
        assert(0 <= l && l <= r && r <= N);
        int ans = 0;
        for (l += N, r += N; l < r; l >>= 1, r >>= 1) {
            if (l & 1) {
                ans += end(st[l]) - lower_bound(begin(st[l]), end(st[l]), key, cmp);
                ++l;
            }
            if (r & 1) {
                --r;
                ans += end(st[r]) - lower_bound(begin(st[r]), end(st[r]), key, cmp);
            }
        }
        return ans;
    }
    // O(log^2 D)
    int count_within(int l, int r, const T& keyl, const T& keyr) const {
        assert(0 <= l && l <= r && r <= N && !cmp(keyr, keyl));
        int ans = 0;
        for (l += N, r += N; l < r; l >>= 1, r >>= 1) {
            if (l & 1) {
                ans += lower_bound(begin(st[l]), end(st[l]), keyr, cmp) -
                       lower_bound(begin(st[l]), end(st[l]), keyl, cmp);
                ++l;
            }
            if (r & 1) {
                --r;
                ans += lower_bound(begin(st[r]), end(st[r]), keyr, cmp) -
                       lower_bound(begin(st[r]), end(st[r]), keyl, cmp);
            }
        }
        return ans;
    }
    // O(SZ log D) where SZ is the size of the answer
    vector<T> get_sorted_list(int l, int r) const {
        assert(0 <= l && l <= r && r <= N);
        vector<T> ans;
        for (l += N, r += N; l < r; l >>= 1, r >>= 1) {
            if (int sz = ans.size(); l & 1) {
                ans.insert(end(ans), begin(st[l]), end(st[l]));
                inplace_merge(begin(ans), begin(ans) + sz, end(ans), cmp);
                ++l;
            }
            if (int sz = ans.size(); r & 1) {
                --r;
                ans.insert(begin(ans), begin(st[r]), end(st[r]));
                inplace_merge(begin(ans), end(ans) - sz, end(ans), cmp);
            }
        }
        return ans;
    }
};
