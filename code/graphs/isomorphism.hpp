#pragma once

#include "hash.hpp"
#include "linear/matrix.hpp"

using edges_t = vector<array<int, 2>>;

// O(V³ log V) topology, has false positives
struct graph_topology {
    using num = modnum<999999893>;
    static inline unordered_map<vector<int>, int, Hasher> node_cache, graph_cache;
    using Edges = vector<array<int, 2>>;
    using Graph = vector<basic_string<int>>;

    static auto make_graph(int V, const Edges& edges) {
        Graph graph(V);
        for (auto [u, v] : edges) {
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
        return graph;
    }

    static auto get_node(const vector<int>& node) {
        if (auto pos = node_cache.find(node); pos != end(node_cache)) {
            return pos->second;
        } else {
            int S = node_cache.size();
            return node_cache[node] = S;
        }
    }

    static auto get_graph(const vector<int>& graph) {
        if (auto pos = graph_cache.find(graph); pos != end(graph_cache)) {
            return pos->second;
        } else {
            int S = graph_cache.size();
            return graph_cache[graph] = S;
        }
    }

    static auto undirected_vertex_hashes(int V, const Edges& edges) {
        mat<num> A({V, V});
        for (auto [u, v] : edges) {
            A[u][v] += 1, A[v][u] += 1;
        }
        A = A ^ V;
        vector<int> ids(V);
        for (int u = 0; u < V; u++) {
            vector<int> vs;
            for (int i = 0; i < V; i++) {
                vs.push_back(int(A[u][i]));
            }
            swap(vs[0], vs[u]);
            sort(begin(vs) + 1, end(vs));
            ids[u] = get_node(vs);
        }
        return ids;
    }

    static auto undirected_graph_hash(int V, const Edges& edges) {
        auto ids = undirected_vertex_hashes(V, edges);
        sort(begin(ids), end(ids));
        return get_graph(ids);
    }
};
