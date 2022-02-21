#pragma once

#include "hash.hpp"
#include "algo/y_combinator.hpp"

/**
 * Unrooted tree hash, safe hash version
 * Complexity: O(V log D), where D<=V is the maximum degree
 */
auto hash_unrooted_tree(int V, const vector<array<int, 2>>& g) {
    static hash<vector<size_t>> vec_hasher;
    static hash<pair<size_t, size_t>> pair_hasher;

    vector<vector<int>> adj(V);
    for (auto [u, v] : g) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    vector<int> subsize(V), centroids;

    auto dfs_centroid = y_combinator([&](auto self, int u, int p) -> void {
        subsize[u] = 1;
        bool is = true;
        for (int v : adj[u]) {
            if (v != p) {
                self(v, u);
                subsize[u] += subsize[v];
                is &= subsize[v] <= V / 2;
            }
        }
        if (is && V - subsize[u] <= V / 2)
            centroids.push_back(u);
    });

    dfs_centroid(0, -1);
    int C = centroids.size();
    assert(C == 1 || C == 2);

    auto hash_subtree = y_combinator([&](auto self, int u, int p) -> size_t {
        vector<size_t> hashes;
        subsize[u] = 1;
        for (int v : adj[u]) {
            if (v != p) {
                hashes.push_back(self(v, u));
                subsize[u] += subsize[v];
            }
        }
        sort(begin(hashes), end(hashes));
        hashes.push_back(subsize[u]);
        return vec_hasher(hashes);
    });

    if (C == 1) {
        return hash_subtree(centroids[0], -1);
    } else {
        auto hu = hash_subtree(centroids[0], centroids[1]);
        auto hv = hash_subtree(centroids[1], centroids[0]);
        pair<size_t, size_t> phash = minmax(hu, hv);
        return pair_hasher(phash);
    }
}

/**
 * Rooted tree hash, safe hash version
 * Complexity: O(V log D), where D<=V is the maximum degree
 */
auto hash_rooted_tree(int V, const vector<array<int, 2>>& g, int root) {
    static hash<vector<size_t>> vec_hasher;

    vector<vector<int>> adj(V);
    for (auto [u, v] : g) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    vector<int> subsize(V);

    auto hash_subtree = y_combinator([&](auto self, int u, int p) -> size_t {
        vector<size_t> hashtable;
        subsize[u] = 1;
        for (int v : adj[u]) {
            if (v != p) {
                hashtable.push_back(self(v, u));
                subsize[u] += subsize[v];
            }
        }
        sort(begin(hashtable), end(hashtable));
        hashtable.push_back(subsize[u]);
        return vec_hasher(hashtable);
    });

    return hash_subtree(root, -1);
}

/**
 * Unrooted tree vertex hash, safe hash version
 * Complexity: O(V log D), where D<=V is the maximum degree
 */
auto hash_unrooted_tree_vertices(int V, const vector<array<int, 2>>& g) {
    static hash<vector<size_t>> vec_hasher;

    vector<vector<int>> adj(V);
    for (auto [u, v] : g) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    vector<int> subsize(V), centroids;

    auto dfs_centroid = y_combinator([&](auto self, int u, int p) -> void {
        subsize[u] = 1;
        bool is = true;
        for (int v : adj[u]) {
            if (v != p) {
                self(v, u);
                subsize[u] += subsize[v];
                is &= subsize[v] <= V / 2;
            }
        }
        if (is && V - subsize[u] <= V / 2)
            centroids.push_back(u);
    });

    dfs_centroid(0, -1);
    int C = centroids.size();
    assert(C == 1 || C == 2);

    vector<size_t> hashtable(V);

    auto hash_subtree = y_combinator([&](auto self, int u, int p) -> size_t {
        vector<size_t> hashes;
        subsize[u] = 1;
        for (int v : adj[u]) {
            if (v != p) {
                hashes.push_back(self(v, u));
                subsize[u] += subsize[v];
            }
        }
        sort(begin(hashes), end(hashes));
        hashes.push_back(subsize[u]);
        hashtable[u] = vec_hasher(hashes);
        return hashtable[u];
    });

    if (C == 1) {
        hash_subtree(centroids[0], -1);
    } else {
        hash_subtree(centroids[0], centroids[1]);
        hash_subtree(centroids[1], centroids[0]);
    }

    return hashtable;
}

/**
 * Rooted tree hash, safe hash version
 * Complexity: O(V log D), where D<=V is the maximum degree
 */
auto hash_rooted_tree_vertices(int V, const vector<array<int, 2>>& g, int root) {
    static hash<vector<size_t>> vec_hasher;

    vector<vector<int>> adj(V);
    for (auto [u, v] : g) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    vector<int> subsize(V);
    vector<size_t> hashtable(V);

    auto hash_subtree = y_combinator([&](auto self, int u, int p = -1) -> size_t {
        vector<size_t> hashes;
        subsize[u] = 1;
        for (int v : adj[u]) {
            if (v != p) {
                hashes.push_back(self(v, u));
                subsize[u] += subsize[v];
            }
        }
        sort(begin(hashes), end(hashes));
        hashes.push_back(subsize[u]);
        hashtable[u] = vec_hasher(hashes);
        return hashtable[u];
    });

    hash_subtree(root, -1);
    return hashtable;
}
