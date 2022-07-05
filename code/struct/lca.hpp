#pragma once

#include "struct/disjoint_set.hpp"
#include "algo/y_combinator.hpp"

struct lca_incremental {
    vector<int> parent, depth, jump;

    explicit lca_incremental(int N = 0) : parent(N, -1), depth(N), jump(N) {}

    lca_incremental(const vector<vector<int>>& tree, int root) {
        int N = tree.size();
        ensure(N);
        dfs_tree_root(tree, root);
        for (int u = 0; u < N; u++) {
            if (u != root && parent[u] == -1) {
                dfs_tree_root(tree, u);
            }
        }
    }

    int num_nodes() const { return parent.size(); }

    int max_depth() const { return *max_element(begin(depth), end(depth)); }

    void add_root(int u) {
        ensure(u + 1);
        parent[u] = u;
        depth[u] = 0;
        jump[u] = u;
    }

    void add_child(int u, int p) {
        ensure(u + 1);
        parent[u] = p;
        depth[u] = depth[p] + 1;
        int t = jump[p];
        jump[u] = depth[p] + depth[jump[t]] == 2 * depth[t] ? jump[t] : p;
    }

    int kth_ancestor(int u, int k) const {
        if (k < 0 || depth[u] < k) {
            return -1;
        }
        int dest = depth[u] - k;
        while (depth[u] > dest) {
            if (depth[jump[u]] < dest) {
                u = parent[u];
            } else {
                u = jump[u];
            }
        }
        return u;
    }

    int below(int u, int a) const { return kth_ancestor(u, depth[u] - depth[a] - 1); }

    int lca(int u, int v) const {
        if (depth[u] < depth[v]) {
            v = kth_ancestor(v, depth[v] - depth[u]);
        } else if (depth[u] > depth[v]) {
            u = kth_ancestor(u, depth[u] - depth[v]);
        }
        while (u != v && depth[u] > 0) {
            if (jump[u] == jump[v]) {
                u = parent[u], v = parent[v];
            } else {
                u = jump[u], v = jump[v];
            }
        }
        return u == v ? u : -1;
    }

    int findroot(int u) const {
        while (depth[u] > 0) {
            u = jump[u];
        }
        return u;
    }

    int dist(int u, int v) const { return depth[u] + depth[v] - 2 * depth[lca(u, v)]; }

    bool conn(int u, int v) const { return findroot(u) == findroot(v); }

    bool is_above(int u, int a) const {
        return depth[u] >= depth[a] && kth_ancestor(u, depth[u] - depth[a]) == a;
    }

    bool on_path(int x, int u, int v) const {
        return is_above(x, lca(u, v)) && (is_above(u, x) || is_above(v, x));
    }

    int kth_on_path(int u, int v, int k) const {
        int a = lca(u, v);
        if (k <= depth[u] - depth[a]) {
            return kth_ancestor(u, k);
        } else if (k -= depth[u] - depth[a]; k <= depth[v] - depth[a]) {
            return kth_ancestor(v, depth[v] - depth[a] - k);
        } else {
            return -1;
        }
    }

    int join_node(int a, int b, int c) const {
        int x = lca(a, b), y = lca(b, c), z = lca(c, a);
        return x ^ y ^ z;
    }

    auto get_path(int u, int v) const {
        int a = lca(u, v);
        int D = depth[u] - depth[a] + 1;
        vector<int> path;
        while (u != a) {
            path.push_back(u), u = parent[u];
        }
        path.push_back(a);
        while (v != a) {
            path.push_back(v), v = parent[v];
        }
        reverse(begin(path) + D, end(path));
        return path;
    }

  private:
    void ensure(int N) {
        if (int S = parent.size(); S < N) {
            parent.resize(N, -1);
            depth.resize(N, 0);
            jump.resize(N, 0);
        }
    }

    void dfs_tree(const vector<vector<int>>& tree, int u, int p) {
        for (int v : tree[u]) {
            if (v != p) {
                add_child(v, u);
                dfs_tree(tree, v, u);
            }
        }
    }

    void dfs_tree_root(const vector<vector<int>>& tree, int root) {
        add_root(root);
        dfs_tree(tree, root, -1);
    }
};

auto lca_tarjan(const vector<vector<int>>& tree, int root,
                const vector<array<int, 2>>& queries) {
    int N = tree.size(), Q = queries.size();

    vector<vector<int>> want(N);
    vector<int> lca(queries.size());

    for (int i = 0; i < Q; i++) {
        auto [u, v] = queries[i];
        if (u == v) {
            lca[i] = u;
        } else {
            want[u].push_back(i);
            want[v].push_back(i);
        }
    }

    vector<bool> color(N);
    disjoint_set dsu(N);

    y_combinator([&](auto self, int u, int p) -> void {
        dsu.reroot(u);
        for (int v : tree[u]) {
            if (v != p) {
                self(v, u);
                dsu.join(u, v);
                dsu.reroot(u);
            }
        }
        color[u] = 1;
        for (int i : want[u]) {
            int v = u ^ queries[i][0] ^ queries[i][1];
            if (color[v]) {
                lca[i] = dsu.find(v);
            }
        }
    })(root, -1);

    return lca;
}
