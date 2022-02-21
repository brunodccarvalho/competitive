#pragma once

#include "algo/y_combinator.hpp"
#include "struct/disjoint_set.hpp"

struct block_cut_tree {
    vector<int> rep, revcut;
    vector<vector<int>> tree, block;
    int A = 0, B = 0; // # cut points, # blocks

    block_cut_tree() = default;
    block_cut_tree(int N, const vector<vector<int>>& adj) { assign(N, adj); }

    void assign(int N, const vector<vector<int>>& adj) {
        rep.assign(N, -1);
        A = B = 0;

        vector<int> cutcount(N), index(N), lowlink(N), stack(N);
        int timer = 1, S = 0;

        auto block_dfs = y_combinator([&](auto self, int u, int p) -> void {
            index[u] = lowlink[u] = timer++;
            stack[S++] = u;

            for (int v : adj[u]) {
                if (v != p) {
                    if (index[v]) {
                        lowlink[u] = min(lowlink[u], index[v]);
                    } else {
                        self(v, u);
                        lowlink[u] = min(lowlink[u], lowlink[v]);

                        if (lowlink[v] >= index[u]) {
                            cutcount[u]++;
                            block.push_back({u});
                            int w = stack[--S];
                            while (w != v) {
                                block[B].push_back(w);
                                w = stack[--S];
                            }
                            block[B].push_back(v);
                            B++;
                        }
                    }
                }
            }

            cutcount[u] -= p == -1 && cutcount[u] > 0;
            A += cutcount[u] > 0;
        });

        // Run dfs to find biconnected blocks and cutpoints
        for (int u = 0; u < N; u++) {
            if (!index[u]) {
                block_dfs(u, -1);
            }
        }

        tree.assign(A + B, {});
        revcut.assign(A + B, -1);

        // Assign cutpoint ids
        for (int u = 0, id = B; u < N; u++) {
            if (cutcount[u]) {
                revcut[id] = u;
                rep[u] = id++;
            }
        }

        // Build tree adjacency
        for (int b = 0; b < B; b++) {
            for (int u : block[b]) {
                if (cutcount[u]) {
                    tree[rep[u]].push_back(b);
                    tree[b].push_back(rep[u]);
                } else {
                    rep[u] = b;
                }
            }
        }
    }

    int num_nodes() const { return A + B; }

    bool is_cutpoint(int u) const { return rep[u] >= B; }
};

auto biconnected_components(const vector<vector<int>>& adj) {
    int N = adj.size();
    vector<int> depth(N, -1), parent(N, -1), cutcount(N), order(N);
    int timer = 0;

    auto dfs = y_combinator([&](auto self, int u) -> void {
        order[timer++] = u;
        for (int v : adj[u]) {
            if (depth[v] == -1) {
                depth[v] = depth[u] + 1;
                parent[v] = u;
                self(v);
                cutcount[u] += cutcount[v];
            } else if (depth[u] < depth[v]) {
                cutcount[u]--;
            } else if (depth[u] > depth[v]) {
                cutcount[u]++;
            }
        }
        cutcount[u]--;
    });

    for (int u = 0; u < N; u++) {
        if (depth[u] == -1) {
            depth[u] = 0;
            dfs(u);
        }
    }

    int B = 0;
    vector<int> bmap(N);
    for (int u : order) {
        if (depth[u] && cutcount[u]) {
            bmap[u] = bmap[parent[u]];
        } else {
            bmap[u] = B++;
        }
    }

    return make_pair(B, move(bmap));
}

template <typename Fn>
auto visit_bridges(const vector<vector<int>>& adj, Fn&& visitor) {
    int V = adj.size();

    vector<int> index(V), lowlink(V);
    int timer = 1;

    auto dfs = y_combinator([&](auto self, int u, int p) -> void {
        index[u] = lowlink[u] = timer++;
        for (int v : adj[u]) {
            if (v != p) {
                if (index[v]) {
                    lowlink[u] = min(lowlink[u], index[v]);
                } else {
                    self(v, u);
                    lowlink[u] = min(lowlink[u], lowlink[v]);
                    if (lowlink[v] > index[u]) {
                        visitor(u, v);
                    }
                }
            }
        }
    });

    for (int u = 0; u < V; u++) {
        if (!index[u]) {
            dfs(u, -1);
        }
    }
}

template <typename Fn>
auto visit_cutpoints(const vector<vector<int>>& adj, Fn&& visitor) {
    int V = adj.size();

    vector<int> index(V), lowlink(V);
    int timer = 1;

    auto dfs = y_combinator([&](auto self, int u, int p) -> void {
        index[u] = lowlink[u] = timer++;
        int children = 0;
        for (int v : adj[u]) {
            if (v != p) {
                if (index[v]) {
                    lowlink[u] = min(lowlink[u], index[v]);
                } else {
                    self(v, u);
                    lowlink[u] = min(lowlink[u], lowlink[v]);
                    if (lowlink[v] >= index[u] && p != -1) {
                        visitor(u); // non-root cutpoint
                    }
                    children++;
                }
            }
            if (children > 1 && p == -1) {
                visitor(u); // root cutpoint
            }
        }
    });

    for (int u = 0; u < V; u++) {
        if (!index[u]) {
            dfs(u, -1);
        }
    }
}
