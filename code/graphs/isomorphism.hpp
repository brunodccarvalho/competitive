#pragma once

#include "hash.hpp"
#include "linear/matrix.hpp"

using edges_t = vector<array<int, 2>>;

/**
 * Compute topological hash of a general graph, based on number of paths of length V
 * between every pair of vertices.
 * Complexity: O(V^3 log(V)) time
 *             O(V^2) memory
 */
auto hash_graph_vertices(int V, const edges_t& g) {
    static hash<vector<size_t>> hasher;
    mat<size_t> m({V, V});
    for (auto [u, v] : g)
        m[u][v] = m[v][u] = 1;
    m = m ^ V;

    vector<size_t> hashtable(V);
    for (int n = 0; n < V; n++) {
        vector<size_t> hashes(m[n], m[n + 1]);
        sort(begin(hashes), end(hashes));
        hashtable[n] = hasher(hashes);
    }
    return hashtable;
}

/**
 * Compute the topological hash of a graph, irrespective of its labels (0-indexed)
 */
size_t hash_graph(int V, const edges_t& g) {
    static hash<vector<size_t>> hasher;
    auto hashtable = hash_graph_vertices(V, g);
    sort(begin(hashtable), end(hashtable));
    hashtable.push_back(V), hashtable.push_back(g.size());
    return hasher(hashtable);
}

/**
 * Isomorphism heuristic for two graphs (0-indexed)
 */
bool isomorphic(int V, const edges_t& g1, const edges_t& g2) {
    return g1.size() == g2.size() && hash_graph(V, g1) == hash_graph(V, g2);
}
