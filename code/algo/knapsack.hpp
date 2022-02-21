#pragma once

#include <bits/stdc++.h>
using namespace std;

// Compute sparse subset sums of numbers in nums
// O(ns), where s is the number of unique sums.
template <typename T>
auto sparse_subset_sum(const vector<T>& nums) {
    int N = nums.size();
    map<T, int> seen;
    seen[T(0)] = -1;

    for (int i = 0; i < N; i++) {
        auto x = nums[i];
        if (x > 0) {
            for (auto it = seen.rbegin(); it != seen.rend(); ++it) {
                seen.emplace(it->first + x, i);
            }
        } else if (x < 0) {
            for (auto it = seen.begin(); it != seen.end(); ++it) {
                seen.emplace(it->first + x, i);
            }
        }
    }

    return seen;
}

// Compute min lexicographic subset that sums to target. O(ns)
template <typename T>
auto sparse_subset_sum(const vector<T>& nums, T target) {
    auto seen = sparse_subset_sum(nums);

    if (!seen.count(target)) {
        return std::nullopt;
    } else {
        vector<int> indices;
        while (target != 0) {
            int i = seen.at(target);
            indices.push_back(i);
            target -= nums[i];
        }
        reverse(begin(indices), end(indices));
        return indices;
    }
}

// Compute dense subset sums of numbers in nums
// O(ns), where S=sum(pos(nums))-sum(neg(nums))
auto dense_subset_sum(const vector<int>& nums) {
    int N = nums.size();
    int neg = 0, pos = 0;
    for (int i = 0; i < N; i++) {
        pos += max(nums[i], 0);
        neg += min(nums[i], 0);
    }

    int S = pos - neg + 1;
    assert(S <= 75'000'000); // sanity check

    vector<int> seen(S, 0); // seen[x + neg] for x
    int* data = seen.data() - neg;
    data[0] = -1;

    for (int i = 0; i < N; i++) {
        auto x = nums[i];
        if (x > 0) {
            for (int s = pos; s >= x + neg; s--) {
                if (!data[s] && data[s - x]) {
                    data[s] = i + 1;
                }
            }
        } else if (x < 0) {
            for (int s = neg; s <= x + pos; s++) {
                if (!data[s] && data[s - x]) {
                    data[s] = i + 1;
                }
            }
        }
    }

    return make_tuple(neg, pos, move(seen));
}

optional<vector<int>> dense_subset_sum(const vector<int>& nums, int target) {
    auto [neg, pos, seen] = dense_subset_sum(nums);

    if (target < neg || pos < target || !seen[target - neg]) {
        return std::nullopt;
    } else {
        vector<int> indices;
        while (target != 0) {
            int i = seen[target - neg] - 1;
            indices.push_back(i);
            target -= nums[i];
        }
        reverse(begin(indices), end(indices));
        return indices;
    }
}

// Maximum value repeated knapsack. O(nW) time, O(cap) space
auto repeated_knapsack(int cap, const vector<int>& weight, const vector<int>& value) {
    int N = weight.size();
    vector<int> dp(cap + 1, 0);
    vector<int> pred(cap + 1, -1);

    for (int w = 1; w <= cap; w++) {
        for (int i = 0; i < N; i++) {
            if (weight[i] <= w && dp[w] < dp[w - weight[i]] + value[i]) {
                dp[w] = dp[w - weight[i]] + value[i];
                pred[w] = i;
            }
        }
    }

    int total = 0, w = cap;
    vector<int> quantity(N, 0);
    while (pred[w] != -1) {
        total += value[pred[w]];
        quantity[pred[w]]++;
        w -= weight[pred[w]];
    }
    return make_pair(total, quantity);
}

// Maximum value 0-1 knapsack. O(nW) time, O(n cap) space
auto unit_knapsack(int cap, const vector<int>& weight, const vector<int>& value) {
    int N = weight.size();
    vector<vector<int>> dp(N + 1, vector<int>(cap + 1, 0));

    for (int i = 0; i < N; i++) {
        for (int w = 0; w < weight[i]; w++) {
            dp[i + 1][w] = dp[i][w];
        }
        for (int w = weight[i]; w <= cap; w++) {
            dp[i + 1][w] = max(dp[i][w], dp[i][w - weight[i]] + value[i]);
        }
    }

    int total = 0;
    vector<bool> quantity(N, false);
    for (int w = cap, i = N - 1; w && i >= 0; i--) {
        if (dp[i + 1][w] != dp[i][w]) {
            quantity[i] = true;
            total += value[i];
            w -= weight[i];
        }
    }
    return make_pair(total, quantity);
}
