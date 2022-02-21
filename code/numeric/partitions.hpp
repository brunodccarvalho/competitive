#pragma once

#include "hash.hpp"
#include "random.hpp"

/**
 * Number of partitions of n into k parts each of size between m and M.
 * Complexity: Potentially O(n^2)
 */
long count_partitions(int n, int k, int m = 1, int M = INT_MAX) {
    static unordered_map<tuple<int, int, int>, long> cntpt_memo;

    M = min(M, n);
    if (m > 0)
        return count_partitions(n - m * k, k, 0, M - m);
    if (n < 0 || k <= 0 || (n + k - 1) / k > M)
        return 0;
    if (k == 1)
        return 1;
    if (cntpt_memo.count({n, k, M}))
        return cntpt_memo.at({n, k, M});

    long cnt = 0;
    for (int x = M; x >= m; x--) {
        cnt += count_partitions(n - x, k - 1, m, x);
    }
    return cntpt_memo[{n, k, M}] = cnt;
}

vector<int> first_choice(int n, int k) {
    vector<int> combination(k);
    iota(begin(combination), end(combination), 0), (void)n;
    return combination;
}

bool next_choice(vector<int>& combination, int n) {
    for (int k = combination.size(), i = k - 1; i >= 0; i--) {
        if (combination[i] <= n - k + i) {
            combination[i]++;
            for (int j = i + 1; j < k; j++) {
                combination[j] = combination[j - 1] + 1;
            }
            return true;
        }
    }
    return false;
}

vector<int> first_unsized_partition(int n) { return vector<int>(n, 1); }

bool next_unsized_partition(vector<int>& a, bool self = true) {
    if (self && int(a.size()) == 1) {
        a.assign(a[0], 1);
        return false;
    }
    int k = a.size() - 1;
    int x = a[k - 1] + 1;
    int y = a[k] - 1;
    k -= 1;
    while (x <= y) {
        a.resize(k + 1);
        a[k] = x;
        y -= x;
        k += 1;
    }
    a.resize(k + 1);
    a[k] = x + y;
    return self || k > 0;
}

vector<int> first_sized_partition(int n, int k) {
    vector<int> a(k, 1);
    a[k - 1] = n - k + 1;
    return a;
}
