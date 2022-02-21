#pragma once

#include "struct/disjoint_set.hpp"
#include "algo/y_combinator.hpp"

// Generic edge-decomposed binary tree.
// Edge e is assigned vertex N+e, and the tree has 2N-1 vertices
auto build_edge_virtual_tree(int N, const vector<array<int, 2>>& tree,
                             vector<int> order) {
    disjoint_set dsu(N);
    reverse(begin(order), end(order));

    vector<int> top(N);
    iota(begin(top), end(top), 0);

    int V = 2 * N - 1;
    vector<array<int, 2>> kids(V, {-1, -1});
    vector<int> parent(V, -1);

    for (int e : order) {
        int u = tree[e][0], v = tree[e][1];
        u = dsu.find(u), v = dsu.find(v);
        dsu.join(u, v);
        int a = top[u], b = top[v];
        kids[N + e] = {a, b};
        top[u] = top[v] = parent[a] = parent[b] = N + e;
    }

    int root = N + order.back();
    return make_tuple(root, move(parent), move(kids));
}

// min edge-decomposed binary tree (i.e. root of tree is minimum weight edge)
template <typename T, typename Compare = less<T>>
auto build_min_edge_virtual_tree(int N, const vector<array<int, 2>>& edges,
                                 const vector<T>& weight,
                                 const Compare& cmp = Compare()) {
    int E = N - 1;
    assert(E == int(edges.size()) && E == int(weight.size()));

    vector<int> order(E);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order),
         [&](int a, int b) { return cmp(weight[a], weight[b]); });

    auto [root, parent, kids] = build_edge_virtual_tree(N, edges, order);

    int V = 2 * N - 1;
    vector<int> intime(V), pretime(V);
    int intimer = 0, pretimer = 0;

    y_combinator([&, &kids = kids](auto self, int i) -> void {
        if (i != -1) {
            pretime[pretimer++] = i;
            self(kids[i][0]);
            intime[intimer++] = i;
            self(kids[i][1]);
        }
    })(root);

    return make_tuple(root, move(parent), move(kids), move(intime), move(pretime));
}
