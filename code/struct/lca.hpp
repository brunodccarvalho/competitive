#pragma once

#include "struct/disjoint_set.hpp"
#include "algo/y_combinator.hpp"

struct lca_schieber_vishkin {
    int N, timer = 0;
    vector<int> preorder, parent, I, A, head, depth;

    static int lowest_one_bit(int n) { return n & -n; }
    static int highest_one_bit(int n) { return n ? 1 << (31 - __builtin_clz(n)) : 0; }

    explicit lca_schieber_vishkin(const vector<vector<int>>& tree, int root = -1)
        : N(tree.size()), preorder(N, -1), parent(N), I(N), A(N), head(N), depth(N) {
        if (root != -1) {
            init_dfs1(tree, root, -1);
            init_dfs2(tree, root, -1, 0);
        }
        for (int u = 0; u < N; u++) {
            if (preorder[u] == -1) {
                init_dfs1(tree, u, -1);
                init_dfs2(tree, u, -1, 0);
            }
        }
    }

    auto get_path(int u, int v) {
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

    int enter_into_strip(int u, int hz) const {
        if (lowest_one_bit(I[u]) == hz)
            return u;
        int hw = highest_one_bit(A[u] & (hz - 1));
        return parent[head[(I[u] & -hw) | hw]];
    }

    int lca(int u, int v) const {
        int hb = I[u] == I[v] ? lowest_one_bit(I[u]) : highest_one_bit(I[u] ^ I[v]);
        int hz = lowest_one_bit(A[u] & A[v] & -hb);
        int eu = enter_into_strip(u, hz);
        int ev = enter_into_strip(v, hz);
        return preorder[eu] < preorder[ev] ? eu : ev;
    }

    int dist(int u, int v) const { return depth[u] + depth[v] - 2 * depth[lca(u, v)]; }

  private:
    void init_dfs1(const vector<vector<int>>& tree, int u, int p) {
        parent[u] = p;
        I[u] = preorder[u] = timer++;
        for (int v : tree[u]) {
            if (v != p) {
                depth[v] = depth[u] + 1;
                init_dfs1(tree, v, u);
                if (lowest_one_bit(I[u]) < lowest_one_bit(I[v])) {
                    I[u] = I[v];
                }
            }
        }
        head[I[u]] = u;
    }

    void init_dfs2(const vector<vector<int>>& tree, int u, int p, int parent) {
        A[u] = parent | lowest_one_bit(I[u]);
        for (int v : tree[u]) {
            if (v != p) {
                init_dfs2(tree, v, u, A[u]);
            }
        }
    }
};

/**
 * LCA on a growing forest. Add roots and new children to existing nodes.
 * Complexity: O(N) construction, true O(1) add, O(log D) lca/ancestor
 */
struct lca_incremental {
    vector<int> parent, depth, jump;

    lca_incremental() = default;
    lca_incremental(int N) : parent(N, -1), depth(N), jump(N) {}
    lca_incremental(const vector<vector<int>>& tree, int root) { add_tree(tree, root); }

    void add_tree(const vector<vector<int>>& tree, int root) {
        add_root(root);
        for (int v : tree[root]) {
            dfs_tree(tree, v, root);
        }
    }

    int num_nodes() const { return parent.size(); }

    void add_root(int u) {
        ensure_new(u);
        parent[u] = u;
        depth[u] = 0;
        jump[u] = u;
    }

    void add_child(int p, int u) {
        ensure_new(u);
        parent[u] = p;
        depth[u] = depth[p] + 1;
        int t = jump[p];
        jump[u] = depth[p] + depth[jump[t]] == 2 * depth[t] ? jump[t] : p;
    }

    int ancestor(int u, int steps) const {
        assert(0 <= steps && steps <= depth[u]);
        int dest = depth[u] - steps;
        while (depth[u] > dest) {
            if (depth[jump[u]] < dest) {
                u = parent[u];
            } else {
                u = jump[u];
            }
        }
        return u;
    }

    // Assumes u and v in the same tree, edit this otherwise
    int lca(int u, int v) const {
        if (depth[u] < depth[v]) {
            v = ancestor(v, depth[v] - depth[u]);
        } else if (depth[u] > depth[v]) {
            u = ancestor(u, depth[u] - depth[v]);
        }
        while (u != v) {
            if (jump[u] == jump[v]) {
                u = parent[u], v = parent[v];
            } else {
                u = jump[u], v = jump[v];
            }
        }
        return u;
    }

    int dist(int u, int v) const { return depth[u] + depth[v] - 2 * depth[lca(u, v)]; }

    bool conn(int u, int v) const {
        return ancestor(u, depth[u]) == ancestor(v, depth[v]);
    }

  private:
    void ensure_new(int N) {
        if (int M = parent.size(); N >= M) {
            parent.resize(N + 1, -1);
            depth.resize(N + 1);
            jump.resize(N + 1);
        } else {
            assert(parent[N] == -1);
        }
    }

    void dfs_tree(const vector<vector<int>>& tree, int u, int p) {
        add_child(p, u);
        for (int v : tree[u]) {
            if (v != p) {
                dfs_tree(tree, v, u);
            }
        }
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
