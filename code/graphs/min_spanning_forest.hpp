#pragma once

#include "struct/disjoint_set.hpp"

using edges_t = vector<array<int, 2>>;

long min_spanning_forest_kruskal(int V, const edges_t& g, const vector<long>& weight) {
    int E = g.size();
    disjoint_set set(V);
    vector<int> edges(E);
    iota(begin(edges), end(edges), 0);
    sort(begin(edges), end(edges),
         [&weight](int e1, int e2) { return weight[e1] < weight[e2]; });

    long msf = 0;
    for (int e : edges) {
        auto [u, v] = g[e];
        if (set.find(u) != set.find(v)) {
            msf += weight[e];
            set.join(u, v);
        }
    }
    return msf;
}

long min_spanning_forest_prim(int V, const edges_t& g, const vector<long>& weight) {
    vector<vector<pair<int, long>>> adj(V);
    for (int e = 0, E = g.size(); e < E; e++) {
        auto [u, v] = g[e];
        adj[u].push_back({v, weight[e]});
        adj[v].push_back({u, weight[e]});
    }

    vector<bool> vis(V, false);
    long msf = 0;
    for (int n = 0; n < V; n++) {
        if (vis[n])
            continue;

        priority_queue<pair<long, int>> Q;
        Q.push({0, n});

        while (!Q.empty()) {
            auto [neg_weight, u] = Q.top();
            Q.pop();
            if (vis[u])
                continue;

            vis[u] = true;
            msf += -neg_weight;
            for (auto [v, w] : adj[u])
                if (!vis[v])
                    Q.push({-w, v});
        }
    }
    return msf;
}

long min_spanning_forest_dense(int V, const vector<vector<long>>& weight) {
    static constexpr long inf = LONG_MAX / 2;
    vector<long> pi = weight[0];
    pi[0] = inf;
    long msf = 0;
    for (int n = 1; n < V; n++) {
        int u = min_element(begin(pi), end(pi)) - begin(pi);
        msf += pi[u], pi[u] = inf;
        for (int v = 0; v < V; v++)
            if (pi[v] != inf)
                pi[v] = min(pi[v], weight[u][v]);
    }
    return msf;
}
