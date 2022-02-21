#include "test_utils.hpp"
#include "flow/mincost_flow.hpp"
#include "lib/graph_generator.hpp"

void speed_test_mincost_flow() {
    vector<int> Vs = {100, 300, 600, 1000, 2000, 5000, 10000 /*, 20000, 30000 */};
    vector<double> pVs = {2.0, 5.0, 8.0, 12.0, 20.0};
    vector<double> as = {-.35, -.15, -.05, -.01, -.001, 0, +.001, +.01, +.05, +.15, +.35};

    vector<tuple<int, double, double>> inputs;
    for (int V : Vs) {
        for (double pV : pVs) {
            for (double a : as) {
                double p = pV / V, E = V * pV;
                if (p <= 1.0 && E <= 100'000) {
                    inputs.push_back({V, pV, a});
                }
            }
        }
    }

    const auto runtime = 180'000ms / inputs.size();
    map<tuple<pair<double, double>, int, stringable>, stringable> table;

    auto make_graph = [](int V, double p, double alpha) {
        auto [g, s, t] = random_geometric_flow_dag_connected(V, p, alpha);
        int E = g.size();
        auto cost = rands_wide<int>(E, 1, 100'000'000, -1);
        auto cap = rands_wide<int>(E, 1, 100'000'000, -1);
        return make_tuple(E, g, s, t, cost, cap);
    };

    for (auto [V, pV, alpha] : inputs) {
        START_ACC(mcmf);
        double p = min(1.0, pV / V);
        int64_t Es = 0, Exp = pV * V;

        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            print_time(now, runtime, "speed mcmf V={} E={} a={:.3f} ({} runs)", V, Exp,
                       alpha, runs);

            auto [E, g, s, t, cost, cap] = make_graph(V, p, alpha);

            ADD_TIME_BLOCK(mcmf) {
                mcmflow<int, int, long, long> g0(V);
                for (int i = 0; i < E; i++) {
                    g0.add(g[i][0], g[i][1], cap[i], cost[i]);
                }
                g0.dag_init(s, t);
                g0.mincost_flow(s, t);
            }

            Es += E;
        }

        table[{{pV, alpha}, V, "mcmf"}] = FORMAT_EACH(mcmf, runs);
        table[{{pV, alpha}, V, "E"}] = format("{:.1f}", 1.0 * Es / runs);
    }

    print_time_table(table, "Mincost maxflow");
}

int main() {
    RUN_BLOCK(speed_test_mincost_flow());
    return 0;
}
