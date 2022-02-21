#pragma once

#include "algo/y_combinator.hpp"

// Standard HLD decomposition of tree/forest. Moves the heavy child to tree[u][0]. O(n)
auto build_heavy_light_decomposition(vector<vector<int>>& tree, int root = 0) {
    int V = tree.size();
    vector<int> subsize(V), heavy(V, -1), parent(V, -1), depth(V);
    vector<int> head(V), tin(V), tout(V);
    int timer = 0;

    auto dfs = y_combinator([&](auto self, int u, int p) -> void {
        subsize[u] = 1;
        int largest = 0;
        for (int& v : tree[u]) {
            if (v != p) {
                parent[v] = u;
                depth[v] = depth[u] + 1;
                self(v, u);
                subsize[u] += subsize[v];
                if (largest < subsize[v]) {
                    largest = subsize[v];
                    heavy[u] = v;
                    swap(tree[u][0], v);
                }
            }
        }
    });

    auto decompose = y_combinator([&](auto self, int u, int h, int p) -> void {
        head[u] = h;
        tin[u] = timer++;
        for (int v : tree[u]) {
            if (v != p) {
                self(v, v == heavy[u] ? h : v, u);
            }
        }
        tout[u] = timer;
    });

    dfs(root, -1);
    decompose(root, root, -1);

    for (int u = 0; u < V; u++) {
        if (subsize[u] == 0) {
            dfs(u, -1);
            decompose(u, u, -1);
        }
    }

    return make_tuple(move(parent), move(depth), move(head), move(tin), move(tout));
}
