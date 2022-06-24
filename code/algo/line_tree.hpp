#pragma once

#include "struct/disjoint_set.hpp"

// Generic edge line tree, returns {u, v, edge index}
auto build_edge_line_tree(int N, const vector<array<int, 2>>& tree, vector<int> order) {
    disjoint_set dsu(N);
    reverse(begin(order), end(order));

    vector<int> left(N), right(N);
    iota(begin(left), end(left), 0);
    iota(begin(right), end(right), 0);

    vector<array<int, 3>> line;

    for (int e : order) {
        int u = tree[e][0], v = tree[e][1];
        u = dsu.find(u), v = dsu.find(v);
        dsu.join(u, v);
        int y = dsu.find(u);
        line.push_back({right[u], left[v], e});
        left[y] = left[u], right[y] = right[v];
    }

    return line;
}

// min edge line array for less<T> (i.e. array retains the minimum weight edge on path)
template <typename T, typename Compare = less<T>>
auto build_min_edge_line(int N, const vector<array<int, 2>>& edges,
                         const vector<T>& weight, const Compare& cmp = Compare()) {
    int E = N - 1;
    assert(E == int(edges.size()) && E == int(weight.size()));

    vector<int> order(E);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order),
         [&](int a, int b) { return cmp(weight[a], weight[b]); });

    auto line = build_edge_line_tree(N, edges, order);

    vector<array<int, 2>> adj(N, {-1, -1});
    for (int i = 0; i < E; i++) {
        auto [u, v, e] = line[i];
        adj[u][adj[u][0] != -1] = adj[v][adj[v][0] != -1] = i;
    }

    vector<int> index(N), edge(E);
    vector<T> value(E);

    for (int s = 0; s < N; s++) {
        if (adj[s][1] == -1) {
            int u = s, j = adj[s][0];
            for (int i = 0; i < E; i++) {
                index[u] = i, edge[i] = line[j][2];
                value[i] = weight[edge[i]];
                u = line[j][u == line[j][0]];
                j = adj[u][j == adj[u][0]];
            }
            assert(j == -1);
            index[u] = N - 1;
            break;
        }
    }

    return make_tuple(move(index), move(edge), move(value));
}
