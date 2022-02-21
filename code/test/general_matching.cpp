#include "test_utils.hpp"
#include "lib/general_matching.hpp"
#include "lib/graph_formats.hpp"
#include "matching/micali_vazirani.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>

using bgraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using matemap_t = std::vector<boost::graph_traits<bgraph>::vertex_descriptor>;

bgraph to_boost(int V, const edges_t& g) {
    bgraph bg(V);
    for (auto [u, v] : g)
        add_edge(u, v, bg);
    return bg;
}

int boost_matching_size(const bgraph& bg) {
    matemap_t mate(num_vertices(bg));
    boost::greedy_matching<bgraph, size_t*>::find_matching(bg, &mate[0]);
    boost::edmonds_maximum_cardinality_matching(bg, &mate[0]);
    int cnt = 0;
    for (auto& mapped : mate)
        cnt += mapped != bgraph::null_vertex();
    return cnt / 2;
}

void stress_test_general_matching() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 50'000, runs) {
        print_time(now, 20s, "stress general matching ({} runs)", runs);

        int V = rand_unif<int>(30, 100);
        int M = min(V / 2, rand_unif<int>(0.1 * V, 0.5 * V));
        double p = rand_unif<double>(3, 15) / V;

        auto g = random_general_matching(V, M, p);
        general_matching_hide_topology(V, g);

        bgraph bg = to_boost(V, g);
        int boo = boost_matching_size(bg);

        micali_vazirani vg(V, g);
        int ans0 = vg.max_matching();

        assert(M == boo && M == ans0 && M >= 1);
    }
}

void speed_test_general_matching() {
    vector<int> Vs = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};
    vector<double> pVs = {2.0, 5.0, 8.0, 12.0, 20.0};
    vector<double> as = {-.35, -.15, -.05, -.01, -.001, 0, +.001, +.01, +.05, +.15, +.35};

    vector<tuple<int, double, double>> inputs;
    for (int V : Vs) {
        for (double pV : pVs) {
            for (double a : as) {
                double p = pV / V, E = V * pV;
                if (p <= 1.0 && E <= 5'000'000) {
                    inputs.push_back({V, pV, a});
                }
            }
        }
    }

    const auto runtime = 360'000ms / inputs.size();
    map<tuple<pair<double, double>, int, stringable>, stringable> table;

    for (auto [V, pV, alpha] : inputs) {
        START_ACC(mv);
        double p = min(1.0, pV / V);
        int64_t Es = 0, Exp = pV * V;

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 1000, runs) {
            print_time(now, runtime,
                       "speed general matching V={} E={} alpha={:.3f} ({} runs)", V, Exp,
                       alpha, runs);

            edges_t g = random_geometric_undirected(V, 2 * p, alpha);
            random_relabel_graph_inplace(V, g);

            ADD_TIME_BLOCK(mv) {
                shuffle(begin(g), end(g), mt);
                micali_vazirani vg(V, g);
                vg.max_matching();
            }

            Es += g.size();
        }

        table[{{pV, alpha}, V, "mv"}] = FORMAT_EACH(mv, runs);
        table[{{pV, alpha}, V, "E"}] = format("{:.1f}", 1.0 * Es / runs);
    }

    print_time_table(table, "General matching");
}

int main() {
    RUN_BLOCK(stress_test_general_matching());
    RUN_BLOCK(speed_test_general_matching());
    return 0;
}
