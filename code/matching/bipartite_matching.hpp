#pragma once

#include <bits/stdc++.h>
using namespace std;

// Simple maximum bipartite matching O(VE), better for not huge matchings
struct bipartite_matching {
    static inline default_random_engine rng = default_random_engine(random_device{}());
    int U, V;
    vector<vector<int>> adj;
    vector<int> mu, mv;

    bipartite_matching(int U, int V) : U(U), V(V), adj(U) { clear(); }

    void add(int u, int v) {
        assert(0 <= u && u < U && 0 <= v && v < V);
        adj[u].push_back(v);
    }

    void shuffle_edges() {
        for (int u = 0; u < U; u++) {
            shuffle(begin(adj[u]), end(adj[u]), rng);
        }
    }

    vector<int> vis;
    int iteration = 0;

    // Run once, augment for u
    bool augment(int u) {
        iteration++;
        return dfs(u);
    }

    // Clear mating of u if it exists
    bool clear(int u) {
        if (mu[u] != -1) {
            mu[u] = mv[mu[u]] = -1;
            return true;
        }
        return false;
    }

    // Clear whole mating, reset vis
    void clear() {
        vis.assign(U, 0);
        mu.assign(U, -1);
        mv.assign(V, -1);
        iteration = 0;
    }

    bool dfs(int u) {
        vis[u] = iteration;
        for (int v : adj[u]) {
            if (mv[v] == -1) {
                mu[u] = v, mv[v] = u;
                return true;
            }
        }
        for (int v : adj[u]) {
            if (vis[mv[v]] != iteration && dfs(mv[v])) {
                mu[u] = v, mv[v] = u;
                return true;
            }
        }
        return false;
    }

    int optimize() {
        vector<int> order(U);
        iota(begin(order), end(order), 0);
        shuffle(begin(order), end(order), rng);
        int new_mates = 0, mates = 0;
        do {
            iteration++, new_mates = 0;
            for (int u : order) {
                new_mates += mu[u] == -1 && dfs(u);
            }
            mates += new_mates;
        } while (new_mates && mates < U && mates < V);
        return mates;
    }

    int max_matching() { return clear(), optimize(); }
};
