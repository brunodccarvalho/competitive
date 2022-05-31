#pragma once

#include "flow/tidal_flow.hpp"

// A circulation adaptor for a maxflow solver (can use any of them)
template <typename Flow = int64_t, typename FlowSum = Flow>
struct circulation {
    using MaxflowSolver = tidal_flow<Flow, FlowSum>;

    explicit circulation(int V = 0) : V(V), supply(V) {}

    int add(int u, int v, Flow lower, Flow upper) {
        assert(0 <= u && u < V && 0 <= v && v < V && lower <= upper);
        return edges.push_back({u, v, lower, upper}), E++;
    }
    int add_node() { return supply.push_back(0), V++; }

    void update_edge(int e, Flow lower, Flow upper) {
        edges[e] = {edges[e].u, edges[e].v, lower, upper};
    }

    void add_supply(int u, FlowSum supply) { this->supply[u] += supply; }
    void add_demand(int u, FlowSum demand) { this->supply[u] -= demand; }
    void set_supply(int u, FlowSum supply) { this->supply[u] = supply; }
    auto get_flow(int e) const { return mf.get_flow(e) + edges[e].lower; }

    // Run for feasibility: return true if a feasible circulation exists
    auto feasible_circulation() {
        mf = MaxflowSolver(V + 2);
        int s = V, t = V + 1;

        vector<FlowSum> excess = supply;
        FlowSum pos = 0, neg = 0;

        for (auto [u, v, lower, upper] : edges) {
            excess[u] -= lower;
            excess[v] += lower;
            mf.add(u, v, upper - lower);
        }
        for (int u = 0; u < V; u++) {
            if (excess[u] > 0) {
                mf.add(s, u, excess[u]);
                pos += excess[u];
            } else if (excess[u] < 0) {
                mf.add(u, t, -excess[u]);
                neg -= excess[u];
            }
        }

        auto f = mf.maxflow(s, t);
        return make_pair(f == pos && f == neg, f);
    }

    // Run for maximum circulation value: return maxflow on given edge
    auto max_circulation(int me) {
        mf = MaxflowSolver(V + 2);
        int s = V, t = V + 1, a = edges[me].v, b = edges[me].u;

        vector<FlowSum> excess = supply;
        FlowSum source = 0, target = 0;

        for (int e = 0; e < E; e++) {
            auto [u, v, lower, upper] = edges[e];
            excess[u] -= lower;
            excess[v] += lower;
            mf.add(u, v, upper - lower);
        }
        for (int u = 0; u < V; u++) {
            if (excess[u] > 0) {
                mf.add(s, u, excess[u]);
                source += excess[u];
            } else if (excess[u] < 0) {
                mf.add(u, t, -excess[u]);
                target -= excess[u];
            }
        }

        auto F = mf.maxflow(s, t);
        if (F < min(source, target)) {
            return make_pair(false, FlowSum(0));
        } else {
            return make_pair(true, mf.maxflow(a, b));
        }
    }

  private:
    int V, E = 0;
    struct Edge {
        int u, v;
        Flow lower, upper;
    };
    vector<Edge> edges;
    vector<FlowSum> supply;
    MaxflowSolver mf;
};
