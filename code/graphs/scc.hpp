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

auto scc_cycle_gcd(const vector<vector<pair<int, int>>>& out, int C, const vector<int>& cmap) {
    using Sum = int;

    int N = out.size();
    vector<Sum> depth(N), dist(N), cycle_gcd(C);
    vector<bool> vis(N);
    vector<int> dsu_next(N), stack;
    iota(begin(dsu_next), end(dsu_next), 0);

    auto find = [&](int u) {
        while (dsu_next[u] != dsu_next[dsu_next[u]]) {
            dist[u] += dist[dsu_next[u]];
            dsu_next[u] = dsu_next[dsu_next[u]];
        }
        return dsu_next[u];
    };
    auto join = [&](int u, int r, int distance) {
        assert(u == dsu_next[u] && r == dsu_next[r]);
        dsu_next[u] = r;
        dist[u] = distance;
    };
    auto dfs = y_combinator([&](auto self, int u, Sum trail) -> void {
        vis[u] = true;
        stack.push_back(u);
        depth[u] = trail;
        auto& g = cycle_gcd[cmap[u]];

        for (auto [v, w] : out[u]) {
            if (cmap[u] != cmap[v]) continue;
            if (!vis[v]) {
                self(v, w + trail);
                continue;
            }
            int x = find(v);

            // we can form a cycle of this length u->v->...->x->...->u
            g = gcd(g, abs(depth[u] - depth[x] + w + dist[v]));

            // compress this cycle into x
            while (stack.back() != x) {
                int y = stack.back();
                stack.pop_back();

                // we have a path v->...->x->...->y, the distance y->x is y->...->u->v->...->x
                join(y, x, depth[u] - depth[y] + w + dist[v]);
            }
        }

        if (stack.back() == u) {
            stack.pop_back();
        }
    });

    for (int u = 0; u < N; u++) {
        if (!vis[u]) dfs(u, 0);
    }

    return make_pair(move(depth), move(cycle_gcd));
}
