#pragma once

#include "struct/disjoint_set.hpp"
#include "struct/integer_heaps.hpp"

/**
 * Compute cost of optimum rooted arborescence, without extracting the underlying tree.
 * By default minimum arborescence; flip costs to get maximum arborescence.
 * Complexity: O(E log V)
 * Reference: kactl
 */
template <typename Cost = long, typename CostSum = Cost>
optional<CostSum> min_arborescence_cost(int V, int root,
                                        const vector<array<int, 2>>& edges,
                                        const vector<Cost>& cost) {
    int E = edges.size();
    disjoint_set dsu(V);
    lazy_skew_int_heaps<CostSum> heaps(V, E);

    for (int e = 0; e < E; e++) {
        int v = edges[e][1];
        heaps.push(v, e, cost[e]);
    }

    CostSum ans = 0;
    vector<int> seen(V, -1);
    vector<int> growth_path(V);
    seen[root] = root;

    for (int s = 0; s < V; s++) {
        int u = s, index = 0;
        while (seen[u] == -1) {
            if (heaps.empty(u)) {
                return nullopt; // no edge can reach u from root
            }
            auto [e, ecost] = heaps.top(u);
            ans += ecost;
            heaps.update(u, -ecost);
            heaps.pop(u);

            growth_path[index++] = u;
            seen[u] = s;
            u = dsu.find(edges[e][0]);

            if (seen[u] == s) {
                int w;
                do {
                    w = growth_path[--index];
                    heaps.merge(dsu.find(u), u, w);
                    u = dsu.find(u);
                } while (dsu.join(u, w));
                seen[u] = -1;
            }
        }
    }

    return ans;
}

/**
 * Compute cost of optimum rooted arborescence and extract the underlying tree.
 * By default minimum arborescence; flip costs to get maximum arborescence.
 * Complexity: O(E log V)
 * Reference: kactl
 */
template <typename Cost = long, typename CostSum = Cost>
optional<pair<CostSum, vector<int>>> min_arborescence(int V, int root,
                                                      const vector<array<int, 2>>& edges,
                                                      const vector<Cost>& cost) {
    int E = edges.size();
    disjoint_set_rollback dsu(V);
    lazy_skew_int_heaps<CostSum> heaps(V, E);

    for (int e = 0; e < E; e++) {
        int v = edges[e][1];
        heaps.push(v, e, cost[e]);
    }

    CostSum ans = 0;
    vector<int> seen(V, -1), in(V, -1);
    vector<int> node_path(V), edge_path(V); // growth path
    list<tuple<int, int, vector<int>>> cycles;
    seen[root] = root;

    for (int s = 0; s < V; s++) {
        int u = s, index = 0;
        while (seen[u] == -1) {
            if (heaps.empty(u)) {
                return nullopt; // no edge can reach u
            }
            auto [e, ecost] = heaps.top(u);
            ans += ecost;
            heaps.update(u, -ecost);
            heaps.pop(u);

            edge_path[index] = e;
            node_path[index] = u;
            index++;
            seen[u] = s;
            u = dsu.find(edges[e][0]);

            if (seen[u] == s) {
                int w, end = index;
                int time = dsu.time();
                do {
                    w = node_path[--index];
                    heaps.merge(dsu.find(u), u, w);
                    u = dsu.find(u);
                } while (dsu.join(u, w));
                seen[u] = -1;

                vector<int> cycle(begin(edge_path) + index, begin(edge_path) + end);
                cycles.emplace_front(u, time, move(cycle));
            }
        }
        for (int i = 0; i < index; i++) {
            int e = edge_path[i], v = edges[e][1];
            in[dsu.find(v)] = e;
        }
    }

    for (const auto& [u, time, component] : cycles) {
        dsu.rollback(time);
        int prev = in[u];
        for (int e : component) {
            int v = edges[e][1];
            in[dsu.find(v)] = e;
        }
        int v = edges[prev][1];
        in[dsu.find(v)] = prev;
    }

    return make_pair(ans, in);
}
