#pragma once

#include "algo/y_combinator.hpp"

auto build_centroid_decomposition(const vector<vector<int>>& tree) {
    int N = tree.size();
    vector<int> parent(N, -1), depth(N, -1), subsize(N);

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

    auto centroid_dfs = y_combinator([&](auto self, int u, int p, int cp, int S) -> P {
        int processed = 0;

        bool changed;
        do {
            changed = false;
            for (int v : tree[u]) {
                while (v != p && depth[v] == -1 && subsize[v] > S / 2) {
                    auto [more, root] = self(v, u, cp, S);
                    subsize[u] -= more;
                    S -= more, processed += more;
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
            centroid_dfs(u, -1, -1, subsize[u]);
        }
    }

    return make_pair(move(parent), move(depth));
}
