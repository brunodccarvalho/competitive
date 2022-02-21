#pragma once

#include "algo/y_combinator.hpp"

struct shallow_forest {
    vector<int> parent, depth, label, roots;

    explicit shallow_forest(const vector<vector<int>>& tree) {
        int N = tree.size();
        parent.assign(N, -1);
        label.assign(N, 0);
        depth.assign(N, 0);

        vector<int> dp(N);
        vector<stack<int>> stacks(lowbits(N + 1));
        stack<int> path;

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

        for (int u = 0; u < N; u++) {
            if (dp[u] == 0) {
                greedy_labeling(u, -1);
                int D = lowbits(dp[u]) - 1;
                decompose(u, -1, D);
                int root = stacks[D].top();
                stacks[D].pop();
                create_chain(dp[u] & ~dp[root], root);
            }
        }

        for (int u = 0; u < N; u++) {
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
    }

    static inline int lowbits(int u) {
        return u == 0 ? 0 : 8 * sizeof(int) - __builtin_clz(u);
    }

    int num_nodes() const { return parent.size(); }

    int ancestor(int u, int steps) const {
        while (steps--)
            u = parent[u];
        return u;
    }

    int below(int u, int a) const { return ancestor(u, depth[u] - depth[a] - 1); }

    int lca(int u, int v) const {
        while (depth[u] > depth[v])
            u = parent[u];
        while (depth[u] < depth[v])
            v = parent[v];
        while (u != v)
            u = parent[u], v = parent[v];
        return u;
    }

    int findroot(int u) const {
        while (parent[u] != -1)
            u = parent[u];
        return u;
    }

    bool conn(int u, int v) const { return findroot(u) == findroot(v); }
};

/**
 * Perform shallow decomp on a forest and aggregate vertex values on paths (non disjoint)
 * Complexity: O(N log N) construction and O(log N) per query (but very fast)
 */
template <typename T, typename BinOp>
struct shallow_sparse_table : shallow_forest {
    vector<vector<T>> table;
    BinOp binop;

    // Vertex aggregates
    shallow_sparse_table(vector<vector<int>>& tree, const vector<T>& val,
                         const BinOp& op = BinOp())
        : shallow_forest(tree), binop(op) {
        int N = num_nodes(), D = *max_element(begin(depth), end(depth));
        table.assign(D + 1, vector<T>(N));

        vector<int> bfs(N), prev(N);

        for (int root = 0; root < N; root++) {
            int d = depth[root];
            table[d][root] = val[root];
            prev[root] = -1;
            bfs[0] = root;
            for (int i = 0, S = 1; i < S; i++) {
                int u = bfs[i];
                for (int v : tree[u]) {
                    if (depth[v] > d && v != prev[u]) {
                        table[d][v] = binop(table[d][u], val[v]);
                        prev[v] = u;
                        bfs[S++] = v;
                    }
                }
            }
        }
    }

    auto query(int u, int v) const {
        int a = lca(u, v), d = depth[a];
        return binop(table[d][u], table[d][v]);
    }
};

/**
 * Perform shallow decomposition on a static forest and aggregate edges values on paths
 * Complexity: O(N log N) construction and O(log N) per query (but very fast)
 */
template <typename T, typename BinOp, typename ReverseFn = std::nullptr_t>
struct shallow_edge_sparse_table : shallow_forest {
    vector<vector<T>> table;
    BinOp binop;
    ReverseFn revop;

    shallow_edge_sparse_table(vector<vector<int>>& tree, const vector<T>& pointval,
                              const vector<vector<pair<int, T>>>& val_tree,
                              const BinOp& op = BinOp(),
                              const ReverseFn& rev = ReverseFn())
        : shallow_forest(tree), binop(op), revop(rev) {
        int N = num_nodes(), D = *max_element(begin(depth), end(depth));
        table.assign(D + 1, vector<T>(N));

        vector<int> bfs(N), prev(N);

        for (int root = 0; root < N; root++) {
            int d = depth[root], S = 0;
            table[d][root] = pointval[root];
            for (auto [v, w] : val_tree[root]) {
                if (depth[v] > d) {
                    table[d][v] = w;
                    prev[v] = root;
                    bfs[S++] = v;
                }
            }
            for (int i = 0; i < S; i++) {
                int u = bfs[i];
                for (auto [v, w] : val_tree[u]) {
                    if (depth[v] > d && v != prev[u]) {
                        prev[v] = u;
                        table[d][v] = binop(table[d][u], w);
                        bfs[S++] = v;
                    }
                }
            }
        }
    }

    auto query(int u, int v) const {
        if (u == v) {
            return table[depth[u]][u];
        } else if (int a = lca(u, v), d = depth[a]; a == u) {
            return table[d][v];
        } else if (a == v) {
            return reverse(table[d][u]);
        } else {
            return binop(reverse(table[d][u]), table[d][v]);
        }
    }

    template <typename U> // TODO C++20: replace with std::identity
    auto&& reverse(U&& v) const {
        if constexpr (is_same_v<ReverseFn, std::nullptr_t>) {
            return forward<U>(v);
        } else {
            return revop(forward<U>(v));
        }
    }
};

/**
 * Perform shallow decomp on a forest and aggregate vertex values on paths (disjoint)
 * Complexity: O(N log N) construction and O(log N) per query (but very fast)
 */
template <typename T, typename BinOp, typename ReverseFn = std::nullptr_t>
struct shallow_disjoint_sparse_table : shallow_forest {
    vector<vector<T>> table;
    BinOp binop;
    ReverseFn revop;

    shallow_disjoint_sparse_table(vector<vector<int>>& tree, const vector<T>& val,
                                  const BinOp& op = BinOp(),
                                  const ReverseFn& rev = ReverseFn())
        : shallow_forest(tree), binop(op), revop(rev) {
        int N = num_nodes(), D = *max_element(begin(depth), end(depth));
        table.assign(D + 1, vector<T>(N));

        vector<int> bfs(N), prev(N);

        for (int root = 0; root < N; root++) {
            int d = depth[root], S = 0;
            table[d][root] = val[root];
            for (int v : tree[root]) {
                if (depth[v] > d) {
                    table[d][v] = val[v];
                    prev[v] = root;
                    bfs[S++] = v;
                }
            }
            for (int i = 0; i < S; i++) {
                int u = bfs[i];
                for (int v : tree[u]) {
                    if (depth[v] > d && v != prev[u]) {
                        prev[v] = u;
                        table[d][v] = binop(table[d][u], val[v]);
                        bfs[S++] = v;
                    }
                }
            }
        }
    }

    auto query(int u, int v) const {
        if (u == v) {
            return table[depth[u]][u];
        } else if (int a = lca(u, v), d = depth[a]; a == u) {
            return binop(table[d][u], table[d][v]);
        } else if (a == v) {
            return binop(reverse(table[d][u]), table[d][v]);
        } else {
            return binop(reverse(table[d][u]), binop(table[d][a], table[d][v]));
        }
    }

    template <typename U> // TODO C++20: replace with std::identity
    auto&& reverse(U&& v) const {
        if constexpr (is_same_v<ReverseFn, std::nullptr_t>) {
            return forward<U>(v);
        } else {
            return revop(forward<U>(v));
        }
    }
};
