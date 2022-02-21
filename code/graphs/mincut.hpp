#pragma once

#include "hash.hpp"

using edges_t = vector<array<int, 2>>;
using cut_t = vector<int>;

/**
 * Stoer-Wagner minimum cut, priority-queue based, for sparse graphs.
 * Complexity: O(VE log(V))
 */
auto stoer_wagner(int V, const edges_t& g, const vector<long>& costs, int a = 0) {
    vector<unordered_set<int>> adj(V);
    unordered_map<pair<int, int>, long> cost;

    for (int e = 0, E = g.size(); e < E; e++) {
        auto [u, v] = g[e];
        adj[u].insert(v), adj[v].insert(u);
        cost[minmax(u, v)] = costs[e];
    }

    using cut_range_t = pair<list<int>::iterator, list<int>::iterator>;
    static constexpr long inf = LONG_MAX;

    long best_cost = inf;
    cut_range_t best_cut;

    vector<list<int>> node(V);
    for (int u = 0; u < V; u++)
        node[u] = {u};

    for (int phase = V - 1; phase > 0; phase--) {
        vector<long> sum(V, 0);
        vector<bool> vis(V, false);
        priority_queue<pair<long, int>> Q;
        Q.push({0, a});

        int s = -1, t = a;

        while (!Q.empty()) {
            int u = Q.top().second;
            Q.pop();
            if (vis[u]) {
                continue;
            }
            vis[u] = true;
            s = t, t = u;
            for (int v : adj[u]) {
                if (!vis[v]) {
                    sum[v] += cost.at(minmax(u, v));
                    Q.push({sum[v], v});
                }
            }
        }

        if (best_cost > sum[t])
            best_cost = sum[t], best_cut = {begin(node[t]), begin(node[s])};

        // merge aggregate nodes s, t
        node[s].splice(begin(node[s]), node[t]);

        // move and merge edges from t into s.
        for (int v : adj[t]) {
            auto vs = minmax(v, s), vt = minmax(v, t);
            if (v != s) {
                if (cost.count(vs)) {
                    cost[vs] += cost.at(vt);
                } else {
                    cost[vs] = cost.at(vt);
                    adj[s].insert(v);
                    adj[v].insert(s);
                }
            }
            adj[v].erase(t);
            cost.erase(vt);
        }

        // free t's adjacency list to prevent memory issues.
        unordered_set<int>().swap(adj[t]);
    }

    cut_t cut(best_cut.first, best_cut.second);
    return pair<long, cut_t>{best_cost, cut};
}

/**
 * Stoer-Wagner minimum cut, matrix-based for dense graphs.
 * Complexity: O(V^3)
 */
auto stoer_wagner(int V, vector<vector<long>> cost, int a = 0) {
    using cut_range_t = pair<list<int>::iterator, list<int>::iterator>;
    static constexpr long inf = LONG_MAX / 2, ninf = LONG_MIN / 2;

    long best_cost = inf;
    cut_range_t best_cut;

    vector<list<int>> node(V);
    for (int u = 0; u < V; u++)
        node[u] = {u};

    for (int phase = 1; phase < V; phase++) {
        auto sum = cost[a];
        int s = -1, t = a;

        for (int i = 0; i < V - phase; i++) {
            sum[t] = ninf; // so it doesn't get selected again
            s = t, t = max_element(begin(sum), end(sum)) - begin(sum);
            for (int j = 0; j < V; j++)
                sum[j] += cost[t][j];
        }

        if (best_cost > sum[t] - cost[t][t])
            best_cost = sum[t] - cost[t][t], best_cut = {begin(node[t]), begin(node[s])};

        // merge aggregate nodes s, t
        node[s].splice(begin(node[s]), node[t]);

        for (int i = 0; i < V; i++)
            cost[s][i] += cost[t][i];
        for (int i = 0; i < V; i++)
            cost[i][s] = cost[s][i];

        cost[a][t] = ninf;
    }

    cut_t cut(best_cut.first, best_cut.second);
    return pair<long, cut_t>{best_cost, cut};
}
