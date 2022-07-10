#pragma once

#include "../struct/min_queue.hpp"
#include "y_combinator.hpp"

// Maximum value repeated knapsack (multiset of items). O(NC) time, O(C) space
template <typename V>
auto naive_repeated_knapsack(int C, const vector<int>& weight, const vector<V>& value) {
    int N = weight.size();
    vector<V> dp(C + 1, 0);
    vector<int> pred(C + 1, -1);

    for (int w = 1; w <= C; w++) {
        for (int i = 0; i < N; i++) {
            if (weight[i] <= w && dp[w] < dp[w - weight[i]] + value[i]) {
                dp[w] = dp[w - weight[i]] + value[i];
                pred[w] = i;
            }
        }
    }

    return make_pair(move(dp), move(pred));
}

// Maximum value once knapsack (set of items). O(NC) time, O(C) space, no recovery
template <typename V>
auto naive_unit_knapsack(int C, const vector<int>& weight, const vector<V>& value) {
    int N = weight.size();
    vector<V> dp(C + 1, 0);

    for (int i = 0; i < N; i++) {
        for (int w = C; w >= weight[i]; w--) {
            if (dp[w] < dp[w - weight[i]] + value[i]) {
                dp[w] = dp[w - weight[i]] + value[i];
            }
        }
    }

    return dp;
}

// Compress equal weights for standard 0/1 subset sum
auto subset_sum_compression(vector<int> weights) {
    map<int, int> ws;
    for (int w : weights) {
        ws[w]++;
    }
    vector<int> compact;
    while (ws.size()) {
        auto [w, c] = *ws.begin();
        ws.erase(ws.begin());
        compact.push_back(w), c--;
        if (c % 2) {
            compact.push_back(w), c--;
        }
        if (c > 0) {
            ws[2 * w] += c / 2;
        }
    }
    return compact;
}

// Determine if each 1<=w<=min(W,C) can be achieved as a 0/1 sum of given weights
auto sqrt_subset_sum_bitset(vector<int> weight) {
    static constexpr int B = 1'000'000;
    weight = subset_sum_compression(weight);
    bitset<B + 1> ok;
    ok[0] = true;
    for (int w : weight) {
        if (w <= B) {
            ok |= ok << w;
        }
    }
    return ok;
}

// Determine if each 1<=w<=min(W,C) can be achieved as a 0/1 sum of given weights
auto sqrt_subset_sum_vector(vector<int> weight) {
    static constexpr int B = 1'000'000;
    int W = min<int64_t>(accumulate(begin(weight), end(weight), 0LL), B);
    weight = subset_sum_compression(weight);
    vector<bool> ok(W + 1);
    ok[0] = true;
    for (int w : weight) {
        for (int i = W; i >= w; i--) {
            ok[i] = ok[i] | ok[i - w];
        }
    }
    return ok;
}

// Determine for each 1<=w<=min(W,C) maximum possible value
template <typename V>
auto repeated_window_knapsack(const vector<int>& weight, const vector<V>& value,
                              const vector<int>& copies) {
    static constexpr int B = 1'000'001;
    struct window_max {
        inline V operator()(V a, V b) const { return max<V>(a, b); }
    };

    int N = weight.size();
    int64_t P = 0;
    for (int i = 0; i < N; i++) {
        P += 1LL * weight[i] * copies[i];
    }
    int W = min<int64_t>(P, B);
    vector<V> dp(W + 1);
    dp[0] = 0;
    for (int i = 0; i < N; i++) {
        for (int s = 0; s < weight[i]; s++) {
            window_queue<V, window_max> window;
            for (int w = s, c = 0; w <= W; w += weight[i], c++) {
                window.push(dp[w] - c * value[i]);
                dp[w] = max(dp[w], window.top() + c * value[i]);
                if (c >= copies[i]) {
                    window.pop();
                }
            }
        }
    }
    return dp;
}
