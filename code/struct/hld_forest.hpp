#pragma once

#include "algo/y_combinator.hpp"

struct hld_forest {
    vector<int> subsize, parent, depth, roots, degree;
    vector<int> heavy; // heavy child of this node, tree[u][0]
    vector<int> head;  // top of heavy path
    vector<int> time;  // index of node in preorder tour with no repetition
    vector<int> preorder;

    explicit hld_forest(vector<vector<int>>& tree, int root = -1) {
        int N = tree.size();
        subsize.assign(N, 0);
        parent.assign(N, -1);
        heavy.assign(N, -1);
        depth.assign(N, 0);
        head.assign(N, 0);
        time.assign(N, 0);
        preorder.assign(N, 0);
        degree.assign(N, 0);
        int timer = 0;

        auto dfs = y_combinator([&](auto self, int u, int p) -> void {
            subsize[u] = 1;
            degree[u] = p != -1;
            int biggest = 0;
            for (int& v : tree[u]) {
                if (v != p) {
                    parent[v] = u;
                    depth[v] = depth[u] + 1;
                    self(v, u);
                    subsize[u] += subsize[v];
                    degree[u]++;
                    if (biggest < subsize[v]) {
                        biggest = subsize[v];
                        heavy[u] = v;
                        swap(tree[u][0], v);
                    }
                }
            }
        });

        auto decompose = y_combinator([&](auto self, int u, int h) -> void {
            head[u] = h;
            preorder[timer] = u;
            time[u] = timer++;
            for (int v : tree[u]) {
                if (v != parent[u]) {
                    self(v, v == heavy[u] ? h : v);
                }
            }
        });

        if (root != -1) {
            dfs(root, -1);
            decompose(root, root);
            roots.push_back(root);
        }
        for (int u = 0; u < N; u++) {
            if (subsize[u] == 0) {
                dfs(u, -1);
                decompose(u, u);
                roots.push_back(u);
            }
        }
    }

    int num_nodes() const { return subsize.size(); }

    int kth_ancestor(int u, int k) const {
        assert(0 <= k && k <= depth[u]);
        int dest = depth[u] - k;
        while (depth[u] > dest) {
            if (depth[head[u]] < dest) {
                u = parent[u];
            } else {
                u = head[u];
            }
        }
        return u;
    }

    int below(int u, int a) const { return kth_ancestor(u, depth[u] - depth[a] - 1); }

    int lca(int u, int v) const {
        while (head[u] != head[v]) {
            if (depth[head[u]] > depth[head[v]]) {
                u = parent[head[u]];
            } else {
                v = parent[head[v]];
            }
        }
        return depth[u] < depth[v] ? u : v;
    }

    int findroot(int u) const {
        while (parent[head[u]] != -1) {
            u = parent[head[u]];
        }
        return head[u];
    }

    int dist(int u, int v) const { return depth[u] + depth[v] - 2 * depth[lca(u, v)]; }

    bool conn(int u, int v) const { return findroot(u) == findroot(v); }

    bool is_above(int a, int u) const {
        return time[a] <= time[u] && time[u] < time[a] + subsize[a];
    }

    bool is_above_on_heavy_path(int a, int u) const {
        return head[a] == head[u] && is_above(a, u);
    }

    bool on_path(int x, int u, int v) const {
        return is_above(lca(u, v), x) && (is_above(x, u) || is_above(x, v));
    }

    bool earlier_order(int a, int b) const { return time[a] < time[b]; }

    int kth_on_path(int u, int v, int k) const {
        int a = lca(u, v);
        if (k <= depth[u] - depth[a]) {
            return kth_ancestor(u, k);
        } else {
            return kth_ancestor(v, depth[u] + depth[v] - 2 * depth[a] - k);
        }
    }

    // Centroid and join of three nodes
    int join_node(int a, int b, int c) const {
        int x = lca(a, b), y = lca(b, c), z = lca(c, a);
        return x ^ y ^ z;
    }

    // Split the path from u to v into sorted heavy path segments [l,r), 0<=l<r<=N
    // With merge=true join heavy path segments [l,m) and [m,r) (for efficiency)
    auto vertex_segments(int u, int v, bool merge = false) const {
        vector<array<int, 2>> ranges;
        if (u == v) {
            ranges.push_back({time[u], time[u] + 1});
            return ranges;
        }
        if (time[u] > time[v]) {
            swap(u, v);
        }
        int a = lca(u, v);
        while (depth[head[v]] > depth[a]) {
            ranges.push_back({time[head[v]], time[v] + 1});
            v = parent[head[v]];
        }
        while (depth[head[u]] > depth[a]) {
            ranges.push_back({time[head[u]], time[u] + 1});
            u = parent[head[u]];
        }
        ranges.push_back({time[a], time[u == a ? v : u] + 1});
        reverse(begin(ranges), end(ranges));
        if (merge) {
            int R = ranges.size();
            for (int i = 1, j = 0; i < R; i++) {
                if (ranges[i][0] == ranges[j][1]) {
                    ranges[j][1] = ranges[i][1], R--;
                } else {
                    ranges[++j] = ranges[i];
                }
            }
            ranges.resize(R);
        }
        return ranges;
    }

    // Split the edge path from u to v into sorted heavy path segments [l,r), 1<=l<r<=N
    // We consider edges here, so that vertex u is responsible for its parent edge
    // With merge=true join heavy path segments [l,m) and [m,r) (for efficiency)
    auto edge_segments(int u, int v, bool merge = false) const {
        vector<array<int, 2>> ranges;
        if (u == v) {
            return ranges;
        }
        if (time[u] > time[v]) {
            swap(u, v);
        }
        int a = lca(u, v);
        while (depth[head[v]] > depth[a]) {
            ranges.push_back({time[head[v]], time[v] + 1});
            v = parent[head[v]];
        }
        while (depth[head[u]] > depth[a]) {
            ranges.push_back({time[head[u]], time[u] + 1});
            u = parent[head[u]];
        }
        if (u != a) {
            ranges.push_back({time[a] + 1, time[u] + 1});
        } else if (v != a) {
            ranges.push_back({time[a] + 1, time[v] + 1});
        }
        reverse(begin(ranges), end(ranges));
        if (merge) {
            int R = ranges.size();
            for (int i = 1, j = 0; i < R; i++) {
                if (ranges[i][0] == ranges[j][1]) {
                    ranges[j][1] = ranges[i][1], R--;
                } else {
                    ranges[++j] = ranges[i];
                }
            }
            ranges.resize(R);
        }
        return ranges;
    }

    // Compute a minimal subtree that contains all the nodes with at most 2k-1 nodes
    auto compress_tree(vector<int> nodes) const {
        int k = nodes.size();
        sort(begin(nodes), end(nodes), earlier_order);

        for (int i = 0; i < k - 1; i++) {
            nodes.push_back(lca(nodes[i], nodes[i + 1]));
        }

        sort(begin(nodes) + k, end(nodes), earlier_order);
        inplace_merge(begin(nodes), begin(nodes) + k, end(nodes), earlier_order);
        nodes.erase(unique(begin(nodes), end(nodes)), end(nodes));
        k = nodes.size();

        vector<pair<int, int>> compressed_tree = {{nodes[0], -1}};
        for (int i = 1; i < k; i++) {
            compressed_tree.push_back({nodes[i], lca(nodes[i - 1], nodes[i])});
        }
        return compressed_tree;
    }
};
