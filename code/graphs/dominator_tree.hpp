#pragma once

#include "algo/y_combinator.hpp"

/**
 * Build dominator tree of given graph
 * Nodes 1-indexed (out[0] empty)
 * Source:
 *     "A Fast Algorithm for Finding Dominators in a Flow Graph", T. Lengauer, R. Tarjan
 *      https://tanujkhattar.wordpress.com/2016/01/11/dominator-tree-of-a-directed-graph/
 */
auto build_dominator_tree(int root, const vector<vector<int>>& out) {
    int V = out.size();
    assert(V > 0 && root > 0 && root < V);

    vector<vector<int>> in(V);
    vector<int> dom(V), parent(V), semi(V), vertex(V), ancestor(V), label(V);
    iota(begin(label), end(label), 0);
    int timer = 1;

    for (int u = 1; u < V; u++) {
        for (int v : out[u]) {
            in[v].push_back(u);
        }
    }

    auto dfs_index = y_combinator([&](auto self, int u) -> void {
        vertex[timer] = u, semi[u] = timer++;
        for (int v : out[u]) {
            if (semi[v] == 0) {
                parent[v] = u;
                self(v);
            }
        }
    });

    auto compress = y_combinator([&](auto self, int v) -> void {
        if (ancestor[ancestor[v]] != 0) {
            self(ancestor[v]);
            if (semi[label[v]] > semi[label[ancestor[v]]]) {
                label[v] = label[ancestor[v]];
            }
            ancestor[v] = ancestor[ancestor[v]];
        }
    });

    auto eval = [&](int v) -> int {
        if (ancestor[v] == 0) {
            return v;
        } else {
            compress(v);
            return label[v];
        }
    };

    dfs_index(root);

    vector<int> bucket_head(V, 0), bucket_next(V, 0);

    for (int i = V - 1; i >= 2; i--) {
        int w = vertex[i];
        for (int v : in[w]) {
            int u = eval(v);
            semi[w] = min(semi[w], semi[u]);
        }
        // push w onto the front of bucket b
        int b = vertex[semi[w]];
        bucket_next[w] = bucket_head[b], bucket_head[b] = w;
        ancestor[w] = parent[w]; // link
        // visit all nodes in bucket parent[w]
        for (int v = bucket_head[parent[w]]; v != 0; v = bucket_next[v]) {
            int u = eval(v);
            dom[v] = semi[u] < semi[v] ? u : parent[w];
        }
        bucket_head[parent[w]] = 0;
    }

    for (int i = 2; i < V; i++) {
        int w = vertex[i];
        if (dom[w] != vertex[semi[w]]) {
            dom[w] = dom[dom[w]];
        }
    }
    dom[root] = 0;

    // might wish to return semi as well
    return make_pair(dom, parent);
}
