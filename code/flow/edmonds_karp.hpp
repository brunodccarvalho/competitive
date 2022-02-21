#pragma once

#include <bits/stdc++.h>
using namespace std;

// Edmonds-Karp augmenting paths. O(VE²) easy to modify I guess
template <typename Flow = int64_t, typename FlowSum = Flow>
struct edmonds_karp {
    struct Edge {
        int node[2];
        Flow cap, flow = 0;
    };
    int V, E = 0;
    vector<vector<int>> res;
    vector<Edge> edge;

    explicit edmonds_karp(int V) : V(V), res(V) {}

    int add(int u, int v, Flow capacity) {
        assert(0 <= u && u < V && 0 <= v && v < V && capacity >= 0);
        res[u].push_back(E++), edge.push_back({{u, v}, capacity, 0});
        res[v].push_back(E++), edge.push_back({{v, u}, 0, 0});
        return (E - 2) >> 1;
    }

    vector<int> pred, Q;
    static constexpr Flow flow_inf = numeric_limits<Flow>::max();

    bool bfs(int s, int t) {
        pred.assign(V, -1);
        Q[0] = s, pred[s] = E;
        for (int i = 0, S = 1; i < S && pred[t] == -1; i++) {
            int u = Q[i];
            for (auto e : res[u]) {
                int v = edge[e].node[1];
                if (pred[v] == -1 && v != s && edge[e].flow < edge[e].cap) {
                    pred[v] = e;
                    Q[S++] = v;
                }
            }
        }
        return pred[t] != -1;
    }

    Flow augment(int t) {
        Flow aug_flow = flow_inf;
        for (int e = pred[t]; e != E; e = pred[edge[e].node[0]]) {
            aug_flow = min(aug_flow, edge[e].cap - edge[e].flow);
        }
        for (int e = pred[t]; e != E; e = pred[edge[e].node[0]]) {
            edge[e].flow += aug_flow;
            edge[e ^ 1].flow -= aug_flow;
        }
        return aug_flow;
    }

    FlowSum maxflow(int s, int t) {
        Q.resize(V);
        FlowSum max_flow = 0;
        while (bfs(s, t)) {
            max_flow += augment(t);
        }
        return max_flow;
    }

    Flow get_flow(int e) const { return edge[2 * e].flow; }
    bool left_of_mincut(int u) const { return pred[u] >= 0; }
};
