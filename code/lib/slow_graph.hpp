#pragma once

#include "lib/graph_formats.hpp"
#include "random.hpp"
#include "struct/pbds.hpp"

struct slow_graph {
    // global forest topology
    using tree_pt = list<ordered_set<int>>::iterator;
    int V;
    ordered_set<pair<int, int>> edges; // list of edges
    list<ordered_set<int>> trees;
    vector<tree_pt> tree_of; // tree of each node

    vector<long> val;
    vector<ordered_set<int>> adjacent;

    mutable vector<uint> vis;
    mutable vector<int> bfs;
    mutable uint bfs_timer = 1;

    static inline boold coind = boold(0.5);

    explicit slow_graph(int V)
        : V(V), tree_of(V + 1), val(V + 1), adjacent(V + 1), vis(V + 1), bfs(V + 1) {
        for (int u = 1; u <= V; u++) {
            ordered_set<int> nums;
            nums.insert(u);
            tree_of[u] = trees.insert(end(trees), move(nums));
        }
    }

    int num_nodes() const { return V; }
    int num_edges() const { return edges.size(); }
    int num_components() const { return trees.size(); }
    bool has_edge(int u, int v) const { return edges.find(minmax(u, v)) != end(edges); }

    // ***** MERGE

    int bfs_tree(int u) const {
        int i = 0, S = 1;
        bfs[0] = u, vis[u] = ++bfs_timer;
        while (i < S) {
            for (int v : adjacent[bfs[i++]]) {
                if (vis[v] < bfs_timer) {
                    bfs[S++] = v;
                    vis[v] = bfs_timer;
                }
            }
        }
        return S;
    }

    bool merge_components(int u, int v) {
        if (tree_of[u] != tree_of[v]) {
            if (tree_of[u]->size() < tree_of[v]->size()) {
                swap(u, v);
            }
            auto old = tree_of[v];
            for (int w : *tree_of[v]) {
                tree_of[w] = tree_of[u];
                tree_of[u]->insert(w);
            }
            trees.erase(old);
            return true;
        }
        return false;
    }

    bool fix_components(int u, int v) {
        assert(u != v && tree_of[u] == tree_of[v]);
        if (coind(mt)) {
            swap(u, v);
        }
        int S = bfs_tree(u);
        if (S == int(tree_of[u]->size())) {
            return false;
        }
        ordered_set<int> reachable(begin(bfs), begin(bfs) + S);
        auto new_tree = trees.insert(end(trees), move(reachable));

        for (int i = 0; i < S; i++) {
            tree_of[bfs[i]]->erase(bfs[i]);
            tree_of[bfs[i]] = new_tree;
        }
        return true;
    }

    tree_pt random_tree() const { return tree_of[random_node()]; }

    int random_in_tree(tree_pt tree) const {
        int S = tree->size();
        return *tree->find_by_order(intd(0, S - 1)(mt));
    }

    // ***** UTILS

    int random_node() const { return intd(1, V)(mt); }

    int random_in_tree(int u) const { return random_in_tree(tree_of[u]); }

    int random_adjacent(int u) const {
        int V = adjacent[u].size();
        assert(V > 0);
        return *adjacent[u].find_by_order(intd(0, V - 1)(mt));
    }

    auto random_edge() const {
        int E = edges.size();
        assert(E > 0);
        return *edges.find_by_order(intd(0, E - 1)(mt));
    }

    auto random_non_edge() const {
        int u, v;
        do {
            auto [a, b] = different<int>(1, V);
            tie(u, v) = coind(mt) ? make_pair(a, b) : make_pair(b, a);
        } while (has_edge(u, v));
        return make_pair(u, v);
    }

    auto random_connected() const {
        int u = random_edge().first;
        int v = random_distinct_in_tree(u);
        return make_pair(u, v);
    }

    int random_distinct_in_tree(int u) const {
        int S = bfs_tree(u);
        assert(S > 1);
        int v = bfs[intd(1, S - 1)(mt)];
        return v;
    }

    // ***** CORE

    bool link(int u, int v) {
        assert(u != v && !has_edge(u, v));
        edges.insert(minmax(u, v));
        adjacent[u].insert(v);
        adjacent[v].insert(u);
        return merge_components(u, v);
    }

    bool cut(int u, int v) {
        assert(u != v && has_edge(u, v));
        edges.erase(minmax(u, v));
        adjacent[u].erase(v);
        adjacent[v].erase(u);
        return fix_components(u, v);
    }

    bool conn(int u, int v) const { return tree_of[u] == tree_of[v]; }

    // ***** NODE QUERIES

    auto query_node(int u) const { return val[u]; }

    void update_node(int u, long new_value) { val[u] = new_value; }

    // ***** TREES

    auto query_tree(int u) const {
        return accumulate(begin(*tree_of[u]), end(*tree_of[u]), 0L);
    }

    int tree_size(int u) const { return tree_of[u]->size(); }

    void update_tree(int u, long value) {
        for (int w : *tree_of[u]) {
            val[w] += value;
        }
    }
};
