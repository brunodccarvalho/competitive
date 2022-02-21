#include "test_utils.hpp"
#include "flow/dinitz_flow.hpp"
#include "flow/edmonds_karp.hpp"
#include "flow/tidal_flow.hpp"
#include "flow/circulation.hpp"
#include "flow/classical.hpp"
#include "lib/graph_formats.hpp"
#include "lib/graph_generator.hpp"

template <typename MF, typename Caps>
void add_edges(MF& mf, const edges_t& g, const Caps& caps) {
    int E = g.size();
    for (int i = 0; i < E; i++) {
        mf.add(g[i][0], g[i][1], caps[i]);
    }
}

template <typename T, typename O = T>
auto mid_cap(int V, const edges_t& g, T low, T high, int repulsion) {
    int E = g.size();
    vector<O> cap(E);
    for (int e = 0; e < E; e++) {
        auto [u, v] = g[e];
        int spread = min(abs(u - V / 2) + abs(v - V / 2), V);
        double factor = (1L * spread * spread) / (V * V);
        T actual_high = low + llround(factor * high);
        cap[e] = rand_wide<T, O>(low, actual_high, repulsion);
    }
    return cap;
}

void stress_test_max_flow() {
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

        add_uniform_self_loops(V, g, 0.2);
        auto cap = mid_cap(V, g, 100, 10000, -1);
        return make_tuple(V, g, s, t, cap);
    };

    LOOP_FOR_DURATION_TRACKED_RUNS (4s, now, runs) {
        print_time(now, 4s, "stress maxflow ({} runs)", runs);

        auto [V, g, s, t, cap] = make_graph();

        const int C = 2;
        dinitz_flow<int, int> g1(V);
        tidal_flow<int, int> g2(V);

        add_edges(g1, g, cap);
        add_edges(g2, g, cap);

        vector<int> ans(C);
        ans[0] = g1.maxflow(s, t);
        ans[1] = g2.maxflow(s, t);

        assert(all_eq(ans));
    }
}

void speed_test_max_flow() {
    vector<int> Vs = {100, 300, 600, 1000, 2000, 5000, 10000, 20000, 30000, 50000, 80000};
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

    const auto runtime = 600'000ms / inputs.size();
    map<tuple<pair<int, double>, int, string>, stringable> table;

    for (auto [V, pV, alpha] : inputs) {
        double p = pV / V;
        START_ACC2(dinitz, tidal);
        int64_t Exp = pV * V, Es = 0;

        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            print_time(now, runtime, "speed maxflow V={} E={} a={:.3f} ({} runs)", V, Exp,
                       alpha, runs);

            auto [g, s, t] = random_geometric_flow_connected(V, p, p / 2, alpha);
            add_uniform_self_loops(V, g, 0.1);
            auto cap = mid_cap(V, g, 1, 100'000'000, -10);

            vector<long> ans(2);

            ADD_TIME_BLOCK(dinitz) {
                dinitz_flow<int, long> mf(V);
                for (int i = 0, E = g.size(); i < E; i++) {
                    mf.add(g[i][0], g[i][1], cap[i]);
                }
                ans[0] = mf.maxflow(s, t);
            }

            ADD_TIME_BLOCK(tidal) {
                tidal_flow<int, long> mf(V);
                for (int i = 0, E = g.size(); i < E; i++) {
                    mf.add(g[i][0], g[i][1], cap[i]);
                }
                ans[1] = mf.maxflow(s, t);
            }

            assert(all_eq(ans));
            Es += g.size();
        }

        table[{{pV, alpha}, V, "dinitz"}] = FORMAT_EACH(dinitz, runs);
        table[{{pV, alpha}, V, "tidal"}] = FORMAT_EACH(tidal, runs);
        table[{{pV, alpha}, V, "E"}] = format("{:.1f}", 1.0 * Es / runs);
    }

    print_time_table(table, "Maximum flow");
}

int main() {
    RUN_BLOCK(stress_test_max_flow());
    RUN_BLOCK(speed_test_max_flow());
    return 0;
}
