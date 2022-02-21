#include "test_utils.hpp"
#include "flow/network_simplex.hpp"
#include "lib/graph_generator.hpp"
#include "lib/flow.hpp"

auto generate_killer(int n) {
    assert(3 <= n && n <= 30);
    int V = 2 * n + 2;

    vector<array<int, 2>> graph;
    vector<int64_t> cost, capacity;
    vector<int64_t> supply(V);
    int64_t inf = 2LL << n;

    int s = 0, t = 1;
    supply[s] = inf, supply[t] = -inf;
    // 0  2  3  1
    //    4  5
    //    6  7
    //    8  9

    auto add_edge = [&](int u, int v, int64_t cap, int64_t w) {
        graph.push_back({u, v}), cost.push_back(w), capacity.push_back(cap);
    };
    auto add_block = [&](int u, int64_t w) {
        int v = u + 1;
        for (int i = 2; i < u; i += 2) {
            add_edge(u, i + 1, inf, w);
            add_edge(i, v, inf, w);
        }
    };
    auto p2 = [&](int v) { return 1LL << v; };
    add_edge(s, 2, 1, 0), add_edge(3, t, 2, 0);
    add_edge(s, 4, 3, 0), add_edge(5, t, 2, 0);
    add_edge(s, 6, 5, 0), add_edge(7, t, 5, 0);
    add_edge(2, 3, inf, 0);
    add_block(4, 1), add_block(6, 3);
    for (int i = 4; i <= n; i++) {
        int u = 2 * i, v = 2 * i + 1;
        add_edge(s, u, p2(i - 1) + p2(i - 3), 0);
        add_edge(v, t, p2(i - 1) + p2(i - 3), 0);
        add_block(u, p2(i - 1) - 1);
    }

    int E = graph.size();
    return make_tuple(V, E, move(graph), move(capacity), move(supply), move(cost));
}

void killer_test_network_simplex() {
    for (int n = 3; n <= 25; n++) {
        auto [V, E, g, cap, supply, cost] = generate_killer(n);

        START(ns);
        network_simplex<int64_t, int64_t> ns(V);
        for (int u = 0; u < V; u++) {
            ns.set_supply(u, supply[u]);
        }
        for (int e = 0; e < E; e++) {
            auto [u, v] = g[e];
            ns.add(u, v, 0, cap[e], cost[e]);
        }
        auto F = ns.mincost_flow();
        auto W = ns.get_circulation_cost();
        TIME(ns);
        println("{:>11}ms -- n={} F={} W={}", TIME_MS(ns), n, F, W);
    }
}

void stress_test_network_simplex() {
    auto make_graph = []() {
        int V = rand_wide<int>(10, 100, -4);
        int k = rand_wide<int>(1, V, -1);
        double p = rand_wide<double>(0.05, 0.9, -2);
        double q = rand_wide<double>(0.05, 0.9, -2);
        double alpha = rand_grav<double>(-.9, .9, 2);
        edges_t g;
        int s = 0, t = V - 1;

        discrete_distribution<int> typed({50, 30, 30, 25, 25, 10, 5, 5});
        int type = typed(mt);
        if (type == 0) {
            tie(g, s, t) = random_geometric_flow_connected(V, p, q, alpha);
        } else if (type == 1) {
            tie(g, s, t) = random_geometric_flow_dag_connected(V, p, alpha);
        } else if (type == 2) {
            g = random_geometric_directed(V, p, alpha);
        } else if (type == 3) {
            g = random_uniform_directed_connected(V, p);
        } else if (type == 4) {
            g = cycle_graph(V);
        } else if (type == 5) {
            g = path_graph(V);
        } else if (type == 6) {
            int n = V / k;
            g = k_connected_complete_graph(n, k);
            V = n * k, t = V - 1;
        } else if (type == 7) {
            int n = V / k;
            g = k_connected_complete2_graph(n, k);
            V = n * k, t = V - 1;
        }

        random_relabel_graph_inplace(V, g);
        int E = g.size();
        auto cost = rands_wide<int>(E, -50'000, 100'000, 0);
        auto circulation = generate_feasible_circulation<int>(V, g, {0, 100'000}, -15);
        auto [lower, upper, flow, supply] = circulation;
        return make_tuple(V, E, g, lower, upper, flow, supply, cost);
    };

    LOOP_FOR_DURATION_TRACKED_RUNS (20s, now, runs) {
        print_time(now, 20s, "stress net simplex ({} runs)", runs);

        auto [V, E, g, lower, upper, flow, supply, cost] = make_graph();

        network_simplex<int, long> netw(V);
        for (int u = 0; u < V; u++) {
            netw.add_supply(u, supply[u]);
        }
        for (int e = 0; e < E; e++) {
            auto [u, v] = g[e];
            netw.add(u, v, lower[e], upper[e], cost[e]);
        }

        bool feasible = netw.mincost_circulation();
        assert(feasible);
        netw.verify();
        netw.get_circulation_cost();
    }
}

void speed_test_network_simplex() {
    vector<int> Vs = {100, 300, 600, 1000, 2000, 5000, 10000, 20000, 30000};
    vector<double> pVs = {2.0, 5.0, 8.0, 12.0, 20.0};
    vector<double> as = {-.35, -.15, -.05, -.01, -.001, 0, +.001, +.01, +.05, +.15, +.35};

    vector<tuple<int, double, double>> inputs;
    for (int V : Vs) {
        for (double pV : pVs) {
            for (double alpha : as) {
                double p = pV / V, E = V * pV;
                if (p <= 1.0 && E <= 250'000) {
                    inputs.emplace_back(V, pV, alpha);
                }
            }
        }
    }

    const auto runtime = 300'000ms / inputs.size();
    map<tuple<pair<double, double>, int, stringable>, stringable> table;

    auto make_graph = [](int V, double p, double alpha) {
        edges_t g = random_geometric_directed(V, p, alpha);
        add_uniform_self_loops(V, g, 0.2);
        random_relabel_graph_inplace(V, g);
        int E = g.size();
        auto cost = rands_wide<int>(E, -5'000, 100'000, 0);
        auto circulation = generate_feasible_circulation<int>(V, g, {0, 100'000}, -15);
        auto [lower, upper, flow, supply] = circulation;
        return make_tuple(E, g, lower, upper, flow, supply, cost);
    };

    for (auto [V, pV, alpha] : inputs) {
        double p = pV / V;
        START_ACC(network);
        int64_t Es = 0, Exp = 0;

        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            print_time(now, runtime, "speed net simplex V={} E={} a={:.3f} ({} runs)", V,
                       Exp, alpha, runs);
            auto [E, g, lower, upper, flow, supply, cost] = make_graph(V, p, alpha);

            ADD_TIME_BLOCK(network) {
                network_simplex<int, long> netw(V);
                for (int e = 0; e < E; e++) {
                    netw.add(g[e][0], g[e][1], lower[e], upper[e], cost[e]);
                }
                for (int u = 0; u < V; u++) {
                    netw.add_supply(u, supply[u]);
                }
                bool feasible = netw.mincost_circulation();
                assert(feasible);
                netw.get_circulation_cost();
            }

            Es += E;
        }

        table[{{pV, alpha}, V, "net"}] = FORMAT_EACH(network, runs);
        table[{{pV, alpha}, V, "E"}] = format("{:.1f}", 1.0 * Es / runs);
    }

    print_time_table(table, "Network simplex");
}

int main() {
    RUN_BLOCK(killer_test_network_simplex());
    RUN_BLOCK(stress_test_network_simplex());
    RUN_BLOCK(speed_test_network_simplex());
    return 0;
}
