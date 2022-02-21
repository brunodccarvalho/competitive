#pragma once

#include "hash.hpp"
#include "random.hpp"

using edges_t = vector<array<int, 2>>;

/**
 * Construct adjacency lists
 */
auto make_adjacency_lists_undirected(int V, const edges_t& g) {
    vector<vector<int>> adj(V);
    for (auto [u, v] : g)
        assert(u < V && v < V), adj[u].push_back(v), adj[v].push_back(u);
    return adj;
}

auto make_adjacency_lists_directed(int V, const edges_t& g) {
    vector<vector<int>> adj(V);
    for (auto [u, v] : g)
        assert(u < V && v < V), adj[u].push_back(v);
    return adj;
}

auto make_adjacency_lists_reverse(int V, const edges_t& g) {
    vector<vector<int>> adj(V);
    for (auto [u, v] : g)
        assert(u < V && v < V), adj[v].push_back(u);
    return adj;
}

auto make_adjacency_set_undirected(const edges_t& g) {
    unordered_set<array<int, 2>> adj;
    for (auto [u, v] : g)
        u < v ? adj.insert({u, v}) : adj.insert({v, u});
    return adj;
}

auto make_adjacency_set_directed(const edges_t& g) {
    unordered_set<array<int, 2>> adj;
    for (auto [u, v] : g)
        adj.insert({u, v});
    return adj;
}

auto make_adjacency_set_reverse(const edges_t& g) {
    unordered_set<array<int, 2>> adj;
    for (auto [u, v] : g)
        adj.insert({v, u});
    return adj;
}

/**
 * Check if a graph is (strongly) connected
 */
int count_reachable(const vector<vector<int>>& adj, int s = 0) {
    int i = 0, S = 1, V = adj.size();
    vector<int> bfs{s};
    vector<bool> vis(V, false);
    vis[s] = true;
    while (i++ < S && S < V) {
        for (int v : adj[bfs[i - 1]]) {
            if (!vis[v]) {
                vis[v] = true, S++;
                bfs.push_back(v);
            }
        }
    }
    return S;
}

bool reachable(const vector<vector<int>>& adj, int s, int t) {
    int i = 0, S = 1, V = adj.size();
    vector<bool> vis(V, false);
    vector<int> bfs{s};
    vis[s] = true;
    while (i++ < S && S < V) {
        for (int v : adj[bfs[i - 1]]) {
            if (!vis[v]) {
                vis[v] = true, S++;
                bfs.push_back(v);
                if (v == t)
                    return true;
            }
        }
    }
    return false;
}

bool is_connected_undirected(const edges_t& g, int V) {
    assert(V > 0);
    auto adj = make_adjacency_lists_undirected(V, g);
    return count_reachable(adj) == V;
}

bool is_connected_directed(const edges_t& g, int V) {
    assert(V > 0);
    auto adj = make_adjacency_lists_directed(V, g);
    if (count_reachable(adj) != V)
        return false;
    adj = make_adjacency_lists_reverse(V, g);
    return count_reachable(adj) == V;
}

bool is_rooted_directed(const edges_t& g, int V, int s = 0) {
    assert(V > 0);
    auto adj = make_adjacency_lists_directed(V, g);
    return count_reachable(adj, s) == V;
}
