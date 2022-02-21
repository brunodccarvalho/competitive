#pragma once

#include <bits/stdc++.h>
using namespace std;

// Hopcroft-Karp maximum bipartite matching O(E√V), better for very large matchings
struct hopcroft_karp {
    int U, V;
    vector<vector<int>> adj;
    vector<int> mu, mv;

    hopcroft_karp(int U, int V) : U(U), V(V), adj(U) {}

    void add(int u, int v) {
        assert(0 <= u && u < U && 0 <= v && v < V);
        adj[u].push_back(v);
    }

    vector<int> vis, dist, bfs;
    int iteration;
    static inline constexpr int inf = INT_MAX / 2;

    bool run_bfs() {
        int S = 0;
        for (int u = 0; u < U; u++) {
            if (mu[u] == -1) {
                dist[u] = 0;
                bfs[S++] = u;
            } else {
                dist[u] = inf;
            }
        }
        dist[U] = inf;
        for (int i = 0; i < S; i++) {
            if (int u = bfs[i]; dist[u] < dist[U]) {
                for (int v : adj[u]) {
                    // note: the check v != mu[u] is implicit in dist[mv[v]] == inf
                    if (dist[mv[v]] == inf) {
                        dist[mv[v]] = dist[u] + 1;
                        bfs[S++] = mv[v];
                    }
                }
            }
        }
        return dist[U] != inf;
    }

    bool dfs(int u) {
        if (u == U) {
            return true;
        }
        if (vis[u] == iteration) {
            return false;
        }
        vis[u] = iteration;
        for (int v : adj[u]) {
            if (dist[mv[v]] == dist[u] + 1 && dfs(mv[v])) {
                mv[v] = u;
                mu[u] = v;
                return true;
            }
        }
        return false;
    }

    int max_matching() {
        vis.assign(U + 1, 0);
        dist.assign(U + 1, 0);
        bfs.assign(U + 1, 0);
        mu.assign(U + 1, -1);
        mv.assign(V, U);
        int mates = 0;
        iteration = 0;
        static mt19937 rng(random_device{}());
        for (int u = 0; u < U; u++) {
            shuffle(begin(adj[u]), end(adj[u]), rng);
        }
        while (run_bfs() && mates < U && mates < V) {
            iteration++;
            for (int u = 0; u < U; u++) {
                if (mu[u] == -1 && dfs(u)) {
                    mates++;
                }
            }
        }
        mu.pop_back();
        return mates;
    }
};
