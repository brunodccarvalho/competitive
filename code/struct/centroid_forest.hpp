#pragma once

#include "algo/y_combinator.hpp"

struct centroid_forest {
    vector<int> parent, depth, roots;

    explicit centroid_forest(const vector<vector<int>>& tree) {
        int N = tree.size();
        parent.assign(N, -1);
        depth.assign(N, -1);

        vector<int> subsize(N);

        auto subsize_dfs = y_combinator([&](auto self, int u, int p) -> void {
            subsize[u] = 1;
            for (int v : tree[u]) {
                if (v != p) {
                    self(v, u);
                    subsize[u] += subsize[v];
                }
            }
        });

        using P = pair<int, int>;
        auto dfs = y_combinator([&](auto self, int u, int p, int cp, int S) -> P {
            int processed = 0;

            bool changed;
            do {
                changed = false;
                for (int v : tree[u]) {
                    while (v != p && depth[v] == -1 && subsize[v] > S / 2) {
                        auto [more, root] = self(v, u, cp, S);
                        subsize[u] -= more;
                        S -= more;
                        processed += more;
                        cp = root;
                        changed = true;
                    }
                }
            } while (changed);

            // backtrack if u is not a centroid child of cp; else recurse on children
            if (S - subsize[u] > S / 2) {
                return make_pair(processed, cp);
            }

            parent[u] = cp;
            depth[u] = cp != -1 ? depth[cp] + 1 : 0;

            for (int v : tree[u]) {
                if (v != p && depth[v] == -1) {
                    self(v, u, u, subsize[v]);
                }
            }

            return make_pair(processed + subsize[u], u);
        });

        for (int u = 0; u < N; u++) {
            if (depth[u] == -1) {
                subsize_dfs(u, -1);
                dfs(u, -1, -1, subsize[u]);
            }
            if (parent[u] == -1) {
                roots.push_back(u);
            }
        }
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
 * Perform centroid decomp on a forest and aggregate vertex values on paths (non disjoint)
 * Complexity: O(N log N) construction and O(log N) per query (but very fast)
 */
template <typename T, typename BinOp>
struct centroid_sparse_table : centroid_forest {
    vector<vector<T>> table;
    BinOp binop;

    // Vertex aggregates
    centroid_sparse_table(vector<vector<int>>& tree, const vector<T>& val,
                          const BinOp& op = BinOp())
        : centroid_forest(tree), binop(op) {
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
 * Perform centroid decomposition on a static forest and aggregate edges values on paths
 * Complexity: O(N log N) construction and O(log N) per query (but very fast)
 */
template <typename T, typename BinOp, typename ReverseFn = std::nullptr_t>
struct centroid_edge_sparse_table : centroid_forest {
    vector<vector<T>> table;
    BinOp binop;
    ReverseFn revop;

    centroid_edge_sparse_table(vector<vector<int>>& tree, const vector<T>& pointval,
                               const vector<vector<pair<int, T>>>& val_tree,
                               const BinOp& op = BinOp(),
                               const ReverseFn& rev = ReverseFn())
        : centroid_forest(tree), binop(op), revop(rev) {
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
 * Perform centroid decomp on a forest and aggregate vertex values on paths (disjoint)
 * Complexity: O(N log N) construction and O(log N) per query (but very fast)
 */
template <typename T, typename BinOp, typename ReverseFn = std::nullptr_t>
struct centroid_disjoint_sparse_table : centroid_forest {
    vector<vector<T>> table;
    BinOp binop;
    ReverseFn revop;

    centroid_disjoint_sparse_table(vector<vector<int>>& tree, const vector<T>& val,
                                   const BinOp& op = BinOp(),
                                   const ReverseFn& rev = ReverseFn())
        : centroid_forest(tree), binop(op), revop(rev) {
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
