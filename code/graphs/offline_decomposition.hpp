#include "lca.hpp"
#include "../algo/y_combinator.hpp"

// Compute optimal HLD vertex decomposition for paths known upfront
auto offline_vertex_path_decomposition(vector<vector<int>> &tree,
                                       const vector<array<int, 2>> &paths, int root = 0) {
    int N = tree.size(), P = paths.size();
    lca_schieber_vishkin lca(tree, root);

    // above[u]: how many paths go through u's parent edge
    vector<int> above(N), parent(N, -1);
    for (auto [u, v] : paths) {
        int a = lca.lca(u, v);
        above[a] -= (a != u) + (a != v);
        above[u] += (a != u);
        above[v] += (a != v);
    }

    y_combinator([&](auto self, int u, int p) -> void {
        for (int v : tree[u]) {
            if (v != p) {
                parent[v] = u;
                self(v, u);
                above[u] += above[v];
            }
        }
    })(root, -1);

    // Build offline heavy-light decomposition
    vector<int> heavy(N, -1), depth(N);
    y_combinator([&](auto self, int u, int p) -> void {
        for (int &v : tree[u]) {
            if (v != p) {
                depth[v] = depth[u] + 1;
                self(v, u);
                if (heavy[u] == -1 || above[heavy[u]] < above[v]) {
                    heavy[u] = v;
                    swap(tree[u][0], v);
                }
            }
        }
    })(root, -1);

    vector<int> head(N, -1), time(N);
    int timer = 0;
    y_combinator([&](auto self, int u, int h) -> void {
        head[u] = h;
        time[u] = timer++;
        for (int v : tree[u]) {
            if (v != parent[u]) {
                self(v, v == heavy[u] ? h : v);
            }
        }
    })(root, root);

    // Now decompose the paths
    vector<vector<array<int, 2>>> ranges(P);
    for (int i = 0; i < P; i++) {
        auto [u, v] = paths[i];
        while (head[u] != head[v]) {
            if (depth[head[u]] > depth[head[v]]) {
                ranges[i].push_back({time[head[u]], time[u] + 1});
                u = parent[head[u]];
            } else {
                ranges[i].push_back({time[head[v]], time[v] + 1});
                v = parent[head[v]];
            }
        }
        if (depth[u] < depth[v]) {
            ranges[i].push_back({time[u], time[v] + 1});
        } else {
            ranges[i].push_back({time[v], time[u] + 1});
        }
    }

    // head, heavy, parent, time, above
    return make_tuple(move(time), move(head), ranges);
}

// Compute optimal HLD edge decomposition for paths known upfront
auto offline_edge_path_decomposition(vector<vector<int>> &tree,
                                     const vector<array<int, 2>> &paths, int root = 0) {
    int N = tree.size(), P = paths.size();
    lca_schieber_vishkin lca(tree, root);

    // above[u]: how many paths go through u's parent edge
    vector<int> above(N), parent(N, -1);
    for (auto [u, v] : paths) {
        int a = lca.lca(u, v);
        above[a] -= (a != u) + (a != v);
        above[u] += (a != u);
        above[v] += (a != v);
    }

    y_combinator([&](auto self, int u, int p) -> void {
        for (int v : tree[u]) {
            if (v != p) {
                parent[v] = u;
                self(v, u);
                above[u] += above[v];
            }
        }
    })(root, -1);

    // Build offline heavy-light decomposition
    vector<int> heavy(N, -1), depth(N);
    y_combinator([&](auto self, int u, int p) -> void {
        for (int &v : tree[u]) {
            if (v != p) {
                depth[v] = depth[u] + 1;
                self(v, u);
                if (heavy[u] == -1 || above[heavy[u]] < above[v]) {
                    heavy[u] = v;
                    swap(tree[u][0], v);
                }
            }
        }
    })(root, -1);

    vector<int> head(N, -1), time(N);
    int timer = 0;
    y_combinator([&](auto self, int u, int h) -> void {
        head[u] = h;
        time[u] = timer++;
        for (int v : tree[u]) {
            if (v != parent[u]) {
                self(v, v == heavy[u] ? h : v);
            }
        }
    })(root, root);

    // Now decompose the paths
    vector<vector<array<int, 2>>> ranges(P);
    for (int i = 0; i < P; i++) {
        auto [u, v] = paths[i];
        while (head[u] != head[v]) {
            if (depth[head[u]] > depth[head[v]]) {
                ranges[i].push_back({time[head[u]], time[u] + 1});
                u = parent[head[u]];
            } else {
                ranges[i].push_back({time[head[v]], time[v] + 1});
                v = parent[head[v]];
            }
        }
        if (depth[u] < depth[v]) {
            ranges[i].push_back({time[u] + 1, time[v] + 1});
        } else if (depth[v] < depth[u]) {
            ranges[i].push_back({time[v] + 1, time[u] + 1});
        }
    }

    // head, heavy, parent, time, above
    return make_tuple(move(time), move(head), ranges);
}
