#pragma once

#include "flow/tidal_flow.hpp"

// A circulation adaptor for a maxflow solver (can use any of them)
template <typename Flow = int64_t, typename FlowSum = Flow>
struct circulation {
    using MaxflowSolver = tidal_flow<Flow, FlowSum>;

    explicit circulation(int V = 0) : V(V) {}

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

    // Run for feasibility: return true if a feasible circulation exists
    auto feasible_circulation() {
        FlowSum total_supply = 0, total_demand = 0;
        for (int u = 0; u < V; u++) {
            total_supply += max<FlowSum>(supply[u], 0);
            total_demand += max<FlowSum>(-supply[u], 0);
        }
        if (total_supply != total_demand) {
            return false;
        }

        auto [maxflow, sum] = run();
        return maxflow == total_supply;
    }

    // Run for maximum circulation value: return maxflow including lower bounds
    auto max_circulation() {
        auto [maxflow, sum] = run();
        return maxflow + sum;
    }

  private:
    int V, E = 0;
    struct Edge {
        int u, v;
        Flow lower, upper;
    };
    vector<Edge> edges;
    vector<FlowSum> supply;

    auto run() const {
        MaxflowSolver mf(V + 2);
        int s = V, t = V + 1;

        vector<FlowSum> excess = supply;
        FlowSum lower_sum = 0;

        for (auto [u, v, lower, upper] : edges) {
            excess[u] -= lower;
            excess[v] += lower;
            lower_sum += lower;
            mf.add(u, v, upper - lower);
        }
        for (int u = 0; u < V; u++) {
            if (excess[u] > 0) {
                mf.add(s, u, excess[u]);
            } else if (excess[u] < 0) {
                mf.add(u, t, -excess[u]);
            }
        }

        auto max_flow = mf.maxflow(s, t);
        return make_pair(max_flow, lower_sum);
    }
};
