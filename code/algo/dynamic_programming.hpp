#pragma once

#include "struct/tensor.hpp"
#include "numeric/combinatorics.hpp"

// Compute LIS in O(n log n)
// less<T>: strictly increasing subsequence
// less_equal<T>: mon. increasing subsequence
// greater<T>: strictly decreasing subsequence
// greater_equal<T>: mon. decreasing subsequence
template <typename T, typename CmpFn = less<T>>
vector<int> longest_increasing_subsequence(const vector<T>& nums, CmpFn&& cmp = CmpFn()) {
    int N = nums.size();
    vector<int> P(N, 0), M(N + 1, 0);

    int L = 0;
    for (int i = 0; i < N; i++) {
        int lo = 1, hi = L;
        while (lo <= hi) {
            int mid = (lo + hi + 1) / 2;
            if (cmp(nums[M[mid]], nums[i]))
                lo = mid + 1;
            else
                hi = mid - 1;
        }

        P[i] = M[lo - 1];
        M[lo] = i;
        L = max(L, lo);
    }

    vector<T> subsequence(L);
    for (int i = L - 1, k = M[L]; i >= 0; i--) {
        subsequence[i] = nums[k];
        k = P[k];
    }
    return subsequence;
}

// Longest common subsequence of two strings a and b
// O(ab) time, O(a+b) do not recover
template <typename Vec>
int longest_common_subsequence_length(const Vec& a, const Vec& b) {
    int A = a.size(), B = b.size();
    vector<int> next(B + 1, 0), prev(B + 1, 0);

    for (int i = 0; i < A; i++) {
        for (int j = 0; j < B; j++) {
            if (a[i] == b[j]) {
                next[j + 1] = 1 + prev[j];
            } else {
                next[j + 1] = max(next[j], prev[j + 1]);
            }
        }
        swap(next, prev);
    }

    return prev[B];
}

// Longest common subsequence of two strings a and b
// O(ab) time, O(ab) memory to recover
template <typename Vec>
auto longest_common_subsequence(const Vec& a, const Vec& b) {
    int A = a.size(), B = b.size();
    vector<vector<int>> dp(A + 1, vector<int>(B + 1, 0));

    for (int i = 0; i < A; i++) {
        for (int j = 0; j < B; j++) {
            if (a[i] == b[j])
                dp[i + 1][j + 1] = 1 + dp[i][j];
            else
                dp[i + 1][j + 1] = max(dp[i + 1][j], dp[i][j + 1]);
        }
    }

    Vec subsequence;
    int i = A - 1, j = B - 1;
    while (i >= 0 && j >= 0 && dp[i + 1][j + 1]) {
        if (a[i] == b[j]) {
            subsequence.push_back(a[i]), i--, j--;
        } else if (dp[i + 1][j + 1] == dp[i][j + 1]) {
            i--;
        } else {
            j--;
        }
    }
    reverse(begin(subsequence), end(subsequence));
    return subsequence;
}

// Compute minimum number of intervals [ai,bi) to cover [a,b) (greedy). O(n log n)
template <typename T>
optional<vector<int>> minimum_interval_cover(T a, T b, const vector<array<T, 2>>& intv) {
    if (a >= b) {
        return vector<int>();
    }

    int N = intv.size();
    vector<int> index;
    index.reserve(N);
    for (int i = 0; i < N; i++) {
        if (intv[0] < b && a < intv[1]) {
            index.push_back(i);
        }
    }
    sort(begin(index), end(index), [&](int u, int v) { return intv[u] < intv[v]; });

    T R = a;
    int i = 0;
    vector<int> solution;

    while (R < b) {
        pair<int, int> best = {R + 1, -1};
        while (i < N && intv[index[i]][0] <= R) {
            best = max(best, make_pair(intv[index[i]][1], index[i])), i++;
        }
        if (best.second == -1) {
            return std::nullopt;
        }
        R = best.first;
        solution.push_back(best.second);
    }

    return solution;
}

// Compute how many ways there are to order elements of three collections a,b,c
// such that no two adjacent elements are from the same set, |a|<=A,|b|<=B,|c|<=C. O(ABC)
template <typename O = int64_t>
auto non_adjacent_three_sets_count(int A, int B, int C) {
    tensor<O, 3> ways({A + 2, B + 2, C + 2});
    tensor<O, 4> ways_end({A + 2, B + 2, C + 2, 3}, 0);
    ways_end[{1, 0, 0, 0}] = 1;
    ways_end[{0, 1, 0, 1}] = 1;
    ways_end[{0, 0, 1, 2}] = 1;

    for (int a = 0; a <= A; a++) {
        for (int b = 0; b <= B; b++) {
            for (int c = 0; c <= C; c++) {
                ways_end[{a + 1, b, c, 0}] += ways_end[{a, b, c, 1}];
                ways_end[{a + 1, b, c, 0}] += ways_end[{a, b, c, 2}];
                ways_end[{a, b + 1, c, 1}] += ways_end[{a, b, c, 0}];
                ways_end[{a, b + 1, c, 1}] += ways_end[{a, b, c, 2}];
                ways_end[{a, b, c + 1, 2}] += ways_end[{a, b, c, 0}];
                ways_end[{a, b, c + 1, 2}] += ways_end[{a, b, c, 1}];
                ways[{a, b, c}] += ways_end[{a, b, c, 0}];
                ways[{a, b, c}] += ways_end[{a, b, c, 1}];
                ways[{a, b, c}] += ways_end[{a, b, c, 2}];
                // ways[{a, b, c}] *= fac[a] * fac[b] * fac[c];
            }
        }
    }

    return ways;
}

// For every possible subset sum, compute how many subsets of fixed size give this sum.
// Non-negative integers only. O(SN²) time and O(SN) memory
template <typename O = int64_t>
auto dense_subset_sum_sized_count(const vector<int>& nums) {
    int N = nums.size();
    int S = accumulate(begin(nums), end(nums), 0);
    assert(1LL * (S + 1) * (N + 1) <= 50'000'000); // sanity check

    tensor<O, 2> dp({S + 1, N + 1}, 0);
    dp[{0, 0}] = 1;

    for (int i = 0, maxsum = 0; i < N; maxsum += nums[i], i++) {
        assert(nums[i] >= 0);
        for (int size = i; size >= 0; size--) {
            for (int sum = 0; sum <= maxsum; sum++) {
                dp[{sum + nums[i], size + 1}] += dp[{sum, size}];
            }
        }
    }

    return dp;
}

// For every possible subset sum, compute how many subsets give this sum.
// Non-negative integers only. O(SN) time and O(S) memory
template <typename O = int64_t>
auto dense_subset_sum_count(const vector<int>& nums) {
    int N = nums.size();
    int S = accumulate(begin(nums), end(nums), 0);
    assert(1LL * (S + 1) * (N + 1) <= 50'000'000);

    vector<O> dp(S + 1, 0);
    dp[0] = 1;

    for (int i = 0, maxsum = 0; i < N; maxsum += nums[i], i++) {
        for (int sum = maxsum; sum >= 0; sum--) {
            dp[sum + nums[i]] += dp[sum];
        }
    }

    return dp;
}
