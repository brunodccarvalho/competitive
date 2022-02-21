#pragma once

#include "algo/y_combinator.hpp"

inline int lowbits(int u) { return u == 0 ? 0 : 8 * sizeof(int) - __builtin_clz(u); }

// Build shallowest decomposition of tree/forest. O(n)
auto build_shallow_decomposition(const vector<vector<int>>& tree) {
    int V = tree.size();
    vector<int> dp(V), parent(V, -1), label(V);
    vector<stack<int>> stacks(lowbits(V + 1));

    auto greedy_labeling = y_combinator([&](auto self, int u, int p) -> void {
        int seen = 0, twice = 0;
        for (int v : tree[u]) {
            if (v != p) {
                self(v, u);
                twice |= seen & dp[v];
                seen |= dp[v];
            }
        }
        int fix = (1 << lowbits(twice)) - 1;
        dp[u] = 1 + (seen | fix);
        label[u] = __builtin_ctz(dp[u]);
    });

    auto create_chain = [&](int labels, int u) {
        while (labels) {
            int label = lowbits(labels) - 1;
            labels ^= 1 << label;
            int v = stacks[label].top();
            stacks[label].pop();
            parent[v] = u, u = v;
        }
    };

    auto decompose = y_combinator([&](auto self, int u, int p, int D) -> void {
        int T = tree[u].size();
        for (int i = 0; i < T; i++) {
            if (int v = tree[u][i]; v != p) {
                self(v, u, D);
            }
        }
        stacks[label[u]].push(u);
        for (int i = T - 1; i >= 0; i--) {
            if (int v = tree[u][i]; v != p) {
                create_chain(dp[v] & ~dp[u], u);
            }
        }
    });

    for (int u = 0; u < V; u++) {
        if (dp[u] == 0) {
            greedy_labeling(u, -1);
            int D = lowbits(dp[u]) - 1;
            decompose(u, -1, D);
            int root = stacks[D].top();
            stacks[D].pop();
            create_chain(dp[u] & ~dp[root], root);
        }
    }

    // Compute depths on the decomposition
    vector<int> depth(V);
    stack<int> path;

    for (int u = 0; u < V; u++) {
        int v = parent[u];
        while (v != -1 && depth[v] == 0) {
            path.push(v), v = parent[v];
        }
        while (!path.empty()) {
            v = path.top(), path.pop();
            depth[v] = parent[v] == -1 ? 0 : 1 + depth[parent[v]];
        }
        depth[u] = parent[u] == -1 ? 0 : 1 + depth[parent[u]];
    }

    return make_tuple(move(parent), move(depth), move(label));
}
