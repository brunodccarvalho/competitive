#pragma once

#include "algo/y_combinator.hpp"

/**
 * Compute an euler tour (cycle or path) of a directed or undirected graph
 * If all nodes have even degree it computes a cycle, with first edges starting at s.
 * If two nodes have odd degree it computes a path, with first edges starting at s.
 * The edges are u->{v,id}
 * Complexity: O(V + E)
 */
auto build_euler_tour(int s, int E, const vector<vector<pair<int, int>>>& adj) {
    int V = adj.size();
    vector<bool> used(E, false);
    vector<int> cur(V, 0), path;

    auto dfs = y_combinator([&](auto self, int u) -> void {
        int deg = adj[u].size();
        while (cur[u] != deg) {
            auto [v, e] = adj[u][cur[u]++];
            if (!used[e]) {
                used[e] = true;
                self(v);
                path.push_back(e);
            }
        }
    });

    dfs(s);
    assert(int(path.size()) == E);
    reverse(begin(path), end(path));
    return path;
}

auto build_directed_euler_tour(int s, const vector<array<int, 2>>& edges) {
    vector<vector<int>> adj;
    for (int e = 0, E = edges.size(); e < E; e++) {
        auto [u, v] = edges[e];
        adj.resize(max(int(adj.size()), 1 + max(u, v)));
        adj[u].push_back(e);
    }

    int V = adj.size(), E = edges.size();
    vector<bool> used(E, false);
    vector<int> cur(V, 0), path;

    auto dfs = y_combinator([&](auto self, int u) -> void {
        int deg = adj[u].size();
        while (cur[u] != deg) {
            int e = adj[u][cur[u]++];
            int v = edges[e][1];
            if (!used[e]) {
                used[e] = true;
                self(v);
                path.push_back(e);
            }
        }
    });

    dfs(s);
    assert(int(path.size()) == E);
    reverse(begin(path), end(path));
    return path;
}

auto build_undirected_euler_tour(int s, const vector<array<int, 2>>& edges) {
    vector<vector<int>> adj;
    for (int e = 0, E = edges.size(); e < E; e++) {
        auto [u, v] = edges[e];
        adj.resize(max(int(adj.size()), 1 + max(u, v)));
        adj[u].push_back(e);
        adj[v].push_back(e);
    }

    int V = adj.size(), E = edges.size();
    vector<bool> used(E, false);
    vector<int> cur(V, 0), path;
    // vector<pair<int, int>> path;

    auto dfs = y_combinator([&](auto self, int u) -> void {
        int deg = adj[u].size();
        while (cur[u] != deg) {
            int e = adj[u][cur[u]++];
            int v = edges[e][u == edges[e][0]];
            if (!used[e]) {
                used[e] = true;
                self(v);
                path.push_back(e);
                // path.emplace_back(u, e);
            }
        }
    });

    dfs(s);
    assert(int(path.size()) == E);
    reverse(begin(path), end(path));
    return path;
}
