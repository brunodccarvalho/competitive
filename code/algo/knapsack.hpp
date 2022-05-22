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

// Compute min plus convolution c[k] = min_{i+j=k}(a[i]+b[j]), for convex a and b
auto min_plus_convex_minkowski(const vector<int>& a, const vector<int>& b) {
    int N = a.size(), M = b.size();
    for (int i = 1; i + 1 < N; i++) {
        assert(2 * a[i] <= a[i - 1] + a[i + 1]);
    }
    for (int j = 1; j + 1 < M; j++) {
        assert(2 * b[j] <= b[j - 1] + b[j + 1]);
    }
    vector<int> c(N + M - 1);
    int i = 0, j = 0;
    while (i < N - 1 && j < M - 1) {
        c[i + j] = a[i] + b[j];
        a[i + 1] - a[i] <= b[j + 1] - b[j] ? i++ : j++;
    }
    while (i < N - 1) {
        c[i + j] = a[i] + b[j], i++;
    }
    while (j < M - 1) {
        c[i + j] = a[i] + b[j], j++;
    }
    return c;
}

// Compute max plus convolution c[k] = min_{i+j=k}(a[i]+b[j]), for concave a and b
auto max_plus_concave_minkowski(const vector<int>& a, const vector<int>& b) {
    int N = a.size(), M = b.size();
    for (int i = 1; i + 1 < N; i++) {
        assert(2 * a[i] >= a[i - 1] + a[i + 1]);
    }
    for (int j = 1; j + 1 < M; j++) {
        assert(2 * b[j] >= b[j - 1] + b[j + 1]);
    }
    vector<int> c(N + M - 1);
    int i = 0, j = 0;
    while (i < N - 1 && j < M - 1) {
        c[i + j] = a[i] + b[j];
        a[i + 1] - a[i] >= b[j + 1] - b[j] ? i++ : j++;
    }
    while (i < N - 1) {
        c[i + j] = a[i] + b[j], i++;
    }
    while (j < M - 1) {
        c[i + j] = a[i] + b[j], j++;
    }
    return c;
}

// Compute row-minima index for totally monotone f(x,y). Ties not broken deterministically
template <typename Fn>
auto min_smawk(Fn&& f, int R, int C) {
    using vi = vector<int>;
    vi initrow(R), initcol(C);
    iota(begin(initrow), end(initrow), 0);
    iota(begin(initcol), end(initcol), 0);
    return y_combinator([&](auto self, const vi& row, vi col) -> vi {
        int N = row.size(), M = col.size();
        if (N == 1) {
            int ans = *min_element(begin(col), end(col), [&](int a, int b) {
                return make_pair(f(row[0], a), a) < make_pair(f(row[0], b), b);
            });
            return {ans};
        }
        if (N < M) {
            // Reduce number of columns to number of rows
            vector<array<int, 2>> minima;
            for (int c : col) {
                while (!minima.empty()) {
                    auto [r, p] = minima.back();
                    if (f(r, c) < f(r, p)) {
                        minima.pop_back();
                    } else if (r + 1 < N) {
                        minima.push_back({r + 1, c});
                        break;
                    } else {
                        break;
                    }
                }
                if (minima.empty()) {
                    minima.push_back({0, c});
                }
            }
            while (minima.back()[0] >= N) {
                minima.pop_back();
            }
            M = minima.size();
            for (int i = 0; i < M; i++) {
                col[i] = minima[i][1];
            }
        }
        vector<int> even(N / 2), ans(N);
        for (int i = 1; i < N; i += 2) {
            even[i >> 1] = row[i];
        }
        auto even_ans = self(even, col);
        for (int i = 0, it = 0; i < N; i++) {
            if (i % 2) {
                ans[i] = even_ans[i >> 1];
            } else {
                int last = i < N - 1 ? even_ans[i >> 1] : col.back();
                ans[i] = col[it];
                while (it < last) {
                    if (f(row[i], col[++it]) < f(row[i], ans[i])) {
                        ans[i] = col[it];
                    }
                }
            }
        }
        return ans;
    })(initrow, initcol);
}

template <typename Fn>
auto max_smawk(Fn&& f, int R, int C) {
    return min_smawk([&f](auto r, auto c) { return -f(r, c); }, R, C);
}
