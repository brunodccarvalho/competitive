#pragma once

#include "struct/integer_heaps.hpp"

// Edmonds-Karp augmenting paths for mincost flow. O(V+ElogV) or (V²) per augmentation
// For min-cost flow problems with one source, one sink. Requires pairing_int_heap
// Three initializers. You must call *one* before calling a mincost_flow() function
//     dag_init(s, t)           if the graph is a DAG
//     spfa_init(s, t)          if the graph has possibly negative costs
//     dijkstra_init(s, t)      if the graph has non-negative costs
// Usage:
//     int s = ..., t = ...;
//     mcmflow<int, long> mcf(V);
//     for (edges...) { mcf.add(u, v, cap, cost); }
//     bool st_path_exists = mcf.*_init(s, t);
//     auto [flow, cost, augmentations] = mcf.mincost_flow*(s, t, ...);
template <typename Flow = int64_t, typename Cost = int64_t, typename FlowSum = Flow,
          typename CostSum = Cost>
struct mcmflow {
    struct Edge {
        int node[2];
        Flow cap, flow = 0;
        Cost cost;
    };
    int V, E = 0;
    vector<vector<int>> res;
    vector<Edge> edge;

    explicit mcmflow(int V) : V(V), res(V), pi(V, 0), heap(V, dist) {}

    void add(int u, int v, Flow capacity, Cost cost) {
        assert(0 <= u && u < V && 0 <= v && v < V && u != v && capacity > 0);
        res[u].push_back(E++), edge.push_back({{u, v}, capacity, 0, cost});
        res[v].push_back(E++), edge.push_back({{v, u}, 0, 0, -cost});
    }

    using heap_t = pairing_int_heap<less_container<vector<CostSum>>>;
    vector<CostSum> dist, pi;
    vector<int> prev;
    heap_t heap;
    static inline constexpr Flow flowinf = numeric_limits<Flow>::max() / 2;
    static inline constexpr FlowSum flowsuminf = numeric_limits<FlowSum>::max() / 2;
    static inline constexpr CostSum costsuminf = numeric_limits<CostSum>::max() / 3;

    // First augmenting path on a DAG in O(V+E) with topological sort
    bool dag_init(int s, int t) {
        dist.assign(V, costsuminf);
        prev.assign(V, -1);
        dist[s] = 0;

        vector<int> deg(V), bfs(V);
        bfs[0] = s;

        for (int i = 0, S = 1; i < S; i++) {
            int u = bfs[i];
            for (int e : res[u]) {
                if (e % 2 == 0) { // forward edge
                    int v = edge[e].node[1];
                    if (deg[v] == 0) {
                        bfs[S++] = v;
                    }
                    deg[v]++;
                }
            }
        }
        for (int i = 0, S = 1; i < S && bfs[i] != t; i++) {
            int u = bfs[i];
            for (int e : res[u]) {
                if (e % 2 == 0) { // forward edge
                    int v = edge[e].node[1];
                    CostSum w = edge[e].cost;
                    if (edge[e].flow < edge[e].cap && dist[v] > dist[u] + w) {
                        dist[v] = dist[u] + w;
                        prev[v] = e;
                    }
                    if (--deg[v] == 0) {
                        bfs[S++] = v;
                    }
                }
            }
        }

        reprice(t);
        return prev[t] != -1;
    }

    // First augmenting path with SPFA in O(V+E) expected time.
    bool spfa_init(int s, int t) {
        dist.assign(V, costsuminf);
        prev.assign(V, -1);
        dist[s] = 0;

        vector<bool> in_queue(V, false);
        deque<int> Q;
        Q.push_back(s);

        do {
            int u = Q.front();
            Q.pop_front(), in_queue[u] = false;

            for (auto e : res[u]) {
                int v = edge[e].node[1];
                CostSum w = edge[e].cost;
                if (edge[e].flow < edge[e].cap && dist[v] > dist[u] + w) {
                    dist[v] = dist[u] + w;
                    prev[v] = e;
                    if (!in_queue[v]) {
                        if (Q.empty() || dist[v] <= dist[Q.front()]) {
                            Q.push_front(v);
                        } else {
                            Q.push_back(v);
                        }
                        in_queue[v] = true;
                    }
                }
            }
        } while (!Q.empty());

        reprice(t);
        return prev[t] != -1;
    }

    // First augmenting path with dijkstra (also the regular augmentor) in O(E log V)
    bool dijkstra(int s, int t) {
        dist.assign(V, costsuminf);
        prev.assign(V, -1);
        dist[s] = 0;

        heap.push(s);

        do {
            auto u = heap.pop();
            for (int e : res[u]) {
                int v = edge[e].node[1];
                CostSum w = min(dist[u] + pi[u] - pi[v] + edge[e].cost, costsuminf);
                if (edge[e].flow < edge[e].cap && dist[v] > w) {
                    dist[v] = w;
                    prev[v] = e;
                    heap.push_or_improve(v);
                }
            }
        } while (!heap.empty() && heap.top() != t);

        heap.clear();
        reprice(t);
        return prev[t] != -1;
    }

    void reprice(int t) {
        for (int u = 0; u < V; u++) {
            pi[u] += min(dist[u], dist[t]);
        }
    }

    auto path(int v) const {
        vector<int> path;
        while (prev[v] != -1) {
            path.push_back(prev[v]);
            v = edge[prev[v]].node[0];
        }
        return path;
    }

    // Augment until we get F flow, C cost, P augmenting paths, or hit minimum cut.
    auto mincost_flow(int s, int t, FlowSum F, CostSum C, int P) {
        assert(F > 0 && C > 0 && P > 0);
        FlowSum sflow = 0;
        CostSum scost = 0;
        int paths = 0;

        if (prev[t] == -1) {
            return make_tuple(sflow, scost, paths);
        }
        do {
            auto augmenting_path = path(t);
            Flow df = min(F - sflow, FlowSum(flowinf));
            CostSum dc = 0;
            for (int e : augmenting_path) {
                df = min(df, edge[e].cap - edge[e].flow);
                dc += edge[e].cost;
            }
            if (dc > 0 && df > (C - scost) / dc) {
                df = (C - scost) / dc;
                if (df == 0) {
                    break; // can't augment without busting C
                }
            }
            sflow += df;
            scost += df * dc;
            paths++;
            for (int e : augmenting_path) {
                edge[e].flow += df;
                edge[e ^ 1].flow -= df;
            }
        } while (sflow < F && scost < C && paths < P && dijkstra(s, t));

        return make_tuple(sflow, scost, paths);
    }

    auto mincost_flow(int s, int t) {
        return mincost_flow(s, t, flowsuminf, costsuminf, INT_MAX);
    }
    // bound the number of augmenting paths
    auto mincost_flow_bounded_paths(int s, int t, int P) {
        return mincost_flow(s, t, flowsuminf, costsuminf, P);
    }
    // bound the flow from s to t
    auto mincost_flow_bounded_flow(int s, int t, FlowSum F) {
        return mincost_flow(s, t, F, costsuminf, INT_MAX);
    }
    // bound the flow cost from s to t
    auto mincost_flow_bounded_cost(int s, int t, CostSum C) {
        return mincost_flow(s, t, flowsuminf, C, INT_MAX);
    }
    // reset the flow network; you must call *_init() again
    void clear_flow() {
        for (int e = 0; e < E; e++) {
            edge[e].flow = 0;
        }
        pi.assign(V, 0);
    }

    Flow get_flow(int e) const { return edge[2 * e].flow; }
    bool left_of_mincut(int u) const { return dist[u] < costsuminf; }
};
