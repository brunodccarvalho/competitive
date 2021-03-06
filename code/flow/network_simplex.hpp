#pragma once

#include "../struct/integer_lists.hpp" // linked_lists

/**
 * Network simplex for minimum cost circulation with fixed supply/demand at nodes
 * Supports edge lower bounds, negative costs and negative cost cycles
 *
 * Flow type should be large enough to hold node supplies and edge flows
 * Cost type should be large enough to hold costs and potentials (usually >=64 bits)
 * CostSum type should be large enough to hold inner product of capacities and costs
 *
 * Complexity: O(V) expected per pivot, O(E) worst case
 * Always faster than push relabel for sparse graphs.
 *
 * Usage:
 *   network_simplex<int, long> netw(V);
 *   for (edges...) {
 *       netw.add(u, v, lower, upper, unit_cost);
 *   }
 *   for (nodes...) {
 *       netw.add_supply(u, supply);
 *   }
 *   bool feasible = netw.mincost_circulation();
 *   auto min_cost = netw.get_circulation_cost();
 *
 * References:
 *   LEMON network_simplex.h
 *   OCW MIT MIT15_082JF10_av16.pdf
 *   OCW MIT MIT15_082JF10_lec16.pdf
 */
template <typename Flow = long, typename Cost = long, typename CostSum = int64_t>
struct network_simplex {
    explicit network_simplex(int V) : V(V), node(V + 1) {}

    void add(int u, int v, Flow lower, Flow upper, Cost cost) {
        assert(0 <= u && u < V && 0 <= v && v < V);
        edge.push_back({{u, v}, lower, upper, cost}), E++;
    }

    void add_supply(int u, Flow supply) { node[u].supply += supply; }
    void add_demand(int u, Flow demand) { node[u].supply -= demand; }
    auto get_supply(int u) const { return node[u].supply; }

    auto get_potential(int u) const { return node[u].pi; }
    auto get_flow(int e) const { return edge[e].flow; }
    auto reduced_cost(int e) const {
        auto [u, v] = edge[e].node;
        return edge[e].cost + node[u].pi - node[v].pi;
    }

    auto get_circulation_cost() const {
        CostSum sum = 0;
        for (int e = 0; e < E; e++) {
            sum += edge[e].flow * CostSum(edge[e].cost);
        }
        return sum;
    }

    void verify() const {
        for (int e = 0; e < E; e++) {
            assert(edge[e].lower <= edge[e].flow && edge[e].flow <= edge[e].upper);
            assert(edge[e].flow == edge[e].lower || reduced_cost(e) <= 0);
            assert(edge[e].flow == edge[e].upper || reduced_cost(e) >= 0);
        }
    }

    bool mincost_circulation() {
        static constexpr bool INFEASIBLE = false, OPTIMAL = true;

        Flow sum_supply = 0;
        for (int u = 0; u < V; u++) {
            sum_supply += node[u].supply;
        }
        if (sum_supply != 0) {
            return INFEASIBLE;
        }
        for (int e = 0; e < E; e++) {
            if (edge[e].lower > edge[e].upper) {
                return INFEASIBLE;
            }
        }

        init();
        int in_arc = select_pivot_edge();
        while (in_arc != -1) {
            pivot(in_arc);
            in_arc = select_pivot_edge();
        }

        for (int e = 0; e < E; e++) {
            auto [u, v] = edge[e].node;
            edge[e].flow += edge[e].lower;
            edge[e].upper += edge[e].lower;
            node[u].supply += edge[e].lower;
            node[v].supply -= edge[e].lower;
        }
        for (int e = E; e < E + V; e++) {
            if (edge[e].flow != 0) {
                edge.resize(E);
                return INFEASIBLE;
            }
        }
        edge.resize(E);
        return OPTIMAL;
    }

  private:
    enum ArcState : int8_t { STATE_UPPER = -1, STATE_TREE = 0, STATE_LOWER = 1 };

    struct Node {
        int parent, pred;
        Flow supply;
        Cost pi;
    };
    struct Edge {
        int node[2];
        Flow lower, upper;
        Cost cost;
        Flow flow = 0;
        ArcState state = STATE_LOWER;
    };
    int V, E = 0, next_arc = 0, block_size = 0;
    vector<Node> node;
    vector<Edge> edge;
    linked_lists children;
    vector<int> bfs; // scratchpad for bfs and upwards walk

    void init() {
        // Remove non-zero lower bounds and compute artif_cost as sum of all costs
        Cost artif_cost = 1;
        for (int e = 0; e < E; e++) {
            auto [u, v] = edge[e].node;
            edge[e].flow = 0;
            edge[e].state = STATE_LOWER;
            edge[e].upper -= edge[e].lower;
            node[u].supply -= edge[e].lower;
            node[v].supply += edge[e].lower;
            artif_cost += edge[e].cost < 0 ? -edge[e].cost : edge[e].cost;
        }

        edge.resize(E + V);
        bfs.resize(V + 1);
        children.assign(V + 1, V + 1);
        next_arc = 0;

        // Add root<->node artificial edges with initial supply for feasible flow
        int root = V;
        node[root] = {-1, -1, 0, 0};

        for (int u = 0, e = E; u < V; u++, e++) {
            node[u].parent = root, node[u].pred = e;
            children.push_back(root, u);
            auto supply = node[u].supply;
            if (supply >= 0) {
                node[u].pi = -artif_cost;
                edge[e] = {{u, root}, 0, supply, artif_cost, supply, STATE_TREE};
            } else {
                node[u].pi = artif_cost;
                edge[e] = {{root, u}, 0, -supply, artif_cost, -supply, STATE_TREE};
            }
        }

        block_size = max(int(ceil(sqrt(E + V))), min(10, V + 1));
    }

    int select_pivot_edge() {
        // lemon-like block search, check block_size edges and pick the best one
        Cost minimum = 0;
        int in_arc = -1;
        int count = block_size, seen_edges = E + V;
        for (int &e = next_arc; seen_edges-- > 0; e = e + 1 == E + V ? 0 : e + 1) {
            if (minimum > edge[e].state * reduced_cost(e)) {
                minimum = edge[e].state * reduced_cost(e);
                in_arc = e;
            }
            if (--count == 0 && minimum < 0) {
                break;
            } else if (count == 0) {
                count = block_size;
            }
        }
        return in_arc;
    }

    void pivot(int in_arc) {
        // Find join node (lca)
        auto [u_in, v_in] = edge[in_arc].node;
        int a = u_in, b = v_in;
        while (a != b) {
            a = node[a].parent == -1 ? v_in : node[a].parent;
            b = node[b].parent == -1 ? u_in : node[b].parent;
        }
        int join = a;

        // Orient edge so that we add flow to source->target
        int source = edge[in_arc].state == STATE_LOWER ? u_in : v_in;
        int target = edge[in_arc].state == STATE_LOWER ? v_in : u_in;

        enum OutArcSide { SAME_EDGE, SOURCE_SIDE, TARGET_SIDE };
        Flow flow_delta = edge[in_arc].upper;
        OutArcSide side = SAME_EDGE;
        int u_out = -1;

        // Go up the cycle from source to the join node
        for (int u = source; u != join && flow_delta; u = node[u].parent) {
            int e = node[u].pred;
            bool edge_down = u == edge[e].node[1];
            Flow d = edge_down ? edge[e].upper - edge[e].flow : edge[e].flow;
            if (flow_delta > d) {
                flow_delta = d;
                u_out = u;
                side = SOURCE_SIDE;
            }
        }

        // Go up the cycle from target to the join node
        for (int u = target; u != join && (flow_delta || side != TARGET_SIDE);
             u = node[u].parent) {
            int e = node[u].pred;
            bool edge_up = u == edge[e].node[0];
            Flow d = edge_up ? edge[e].upper - edge[e].flow : edge[e].flow;
            if (flow_delta >= d) {
                flow_delta = d;
                u_out = u;
                side = TARGET_SIDE;
            }
        }

        // Put u_in on the same side as u_out
        u_in = side == SOURCE_SIDE ? source : target;
        v_in = side == SOURCE_SIDE ? target : source;

        // Augment along the cycle
        if (flow_delta) {
            auto delta = edge[in_arc].state * flow_delta;
            edge[in_arc].flow += delta;
            for (int u = edge[in_arc].node[0]; u != join; u = node[u].parent) {
                int e = node[u].pred;
                edge[e].flow += (u == edge[e].node[0] ? -1 : +1) * delta;
            }
            for (int u = edge[in_arc].node[1]; u != join; u = node[u].parent) {
                int e = node[u].pred;
                edge[e].flow += (u == edge[e].node[0] ? +1 : -1) * delta;
            }
        }

        if (side == SAME_EDGE) {
            edge[in_arc].state = ArcState(-edge[in_arc].state);
            return;
        }

        // Replace out_arc with in_arc in the spanning tree
        int out_arc = node[u_out].pred;
        edge[in_arc].state = STATE_TREE;
        edge[out_arc].state = edge[out_arc].flow ? STATE_UPPER : STATE_LOWER;

        // Walk up from u_in to u_out, then fix parent/pred/child pointers backwrads
        int i = 0, S = 0;
        for (int u = u_in; u != u_out; u = node[u].parent) {
            bfs[S++] = u;
        }
        for (i = S - 1; i >= 0; i--) {
            int u = bfs[i], p = node[u].parent;
            children.erase(p);
            children.push_back(u, p);
            node[p].parent = u;
            node[p].pred = node[u].pred;
        }
        children.erase(u_in);
        children.push_back(v_in, u_in);
        node[u_in].parent = v_in;
        node[u_in].pred = in_arc;

        // Adjust potentials in the subtree of u_in (pi_delta is not 0).
        Cost current_pi = reduced_cost(in_arc);
        Cost pi_delta = (u_in == edge[in_arc].node[0] ? -1 : +1) * current_pi;

        bfs[0] = u_in;
        for (i = 0, S = 1; i < S; i++) {
            int u = bfs[i];
            node[u].pi += pi_delta;
            FOR_EACH_IN_LINKED_LIST (v, u, children) { bfs[S++] = v; }
        }
    }
};
