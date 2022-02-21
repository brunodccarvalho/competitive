#pragma once

#include "algo/y_combinator.hpp"
#include "hash.hpp"

auto build_scc(const vector<vector<int>>& adj, bool reverse_order = true) {
    int V = adj.size(), C = 0; // C = number of scc

    vector<int> cmap(V, -1), index(V), lowlink(V), stack(V);
    int timer = 1, S = 0;

    auto dfs = y_combinator([&](auto self, int u) -> void {
        index[u] = lowlink[u] = timer++;
        stack[S++] = u;

        for (int v : adj[u]) {
            if (index[v] && cmap[v] == -1) { // <-- skip cmap'd nodes
                lowlink[u] = min(lowlink[u], index[v]);
            } else if (!index[v]) {
                self(v);
                lowlink[u] = min(lowlink[u], lowlink[v]);
            }
        }

        // found a strongly connected component
        if (index[u] == lowlink[u]) {
            int c = C++;
            int v;
            do {
                v = stack[--S];
                cmap[v] = c;
            } while (u != v);
        }
    });

    for (int u = 0; u < V; u++) {
        if (!index[u]) {
            dfs(u);
        }
    }
    if (!reverse_order) {
        for (int u = 0; u < V; u++) {
            cmap[u] = C - 1 - cmap[u];
        }
    }

    return make_pair(C, cmap);
}

auto condensate_sccedges(const vector<vector<int>>& adj, const vector<int>& cmap) {
    int V = adj.size();
    vector<array<int, 2>> edges;
    for (int u = 0; u < V; u++) {
        for (int v : adj[u]) {
            if (cmap[u] != cmap[v]) {
                edges.push_back({cmap[u], cmap[v]});
            }
        }
    }
    sort(begin(edges), end(edges));
    edges.erase(unique(begin(edges), end(edges)), end(edges));
    return edges;
}

auto condensate_scc(int C, const vector<vector<int>>& adj, const vector<int>& cmap) {
    auto edges = condensate_sccedges(adj, cmap);
    vector<vector<int>> sccout(C), sccin(C);
    for (auto [u, v] : edges) {
        sccout[u].push_back(v);
        sccin[v].push_back(u);
    }
    return make_tuple(edges, sccout, sccin);
}
