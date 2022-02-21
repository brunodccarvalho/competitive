#include "test_utils.hpp"
#include "matching/bipartite_matching.hpp"
#include "lib/bipartite_matching.hpp"
#include "matching/hopcroft_karp.hpp"

void stress_test_bipartite_matching() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 1000, runs) {
        print_time(now, 20s, "stress bipartite matching ({} runs)", runs);

        int U = rand_unif<int>(90, 600);
        int V = rand_unif<int>(90, 600);
        int M = rand_unif<int>(1, min(U, V));
        double p = rand_unif<double>(3, 30) / max(U, V);
        auto g = random_bipartite_matching(U, V, M, p);
        bipartite_matching_hide_topology(U, V, g);

        bipartite_matching mm(U, V);
        for (auto [u, v] : g)
            mm.add(u, v);
        mm.shuffle_edges();

        int m0 = mm.max_matching();

        hopcroft_karp hk(U, V);
        for (auto [u, v] : g)
            hk.add(u, v);
        int m1 = hk.max_matching();

        assert(M == m0 && M == m1);
    }
}

void speed_test_bipartite_matching() {
    vector<int> Us = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};
    vector<int> Vs = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};
    vector<double> pVs = {2.0, 5.0, 8.0, 12.0, 20.0};

    vector<tuple<int, int, double>> inputs;
    for (int U : Us) {
        for (int V : Vs) {
            for (double pV : pVs) {
                double p = pV / U, E = U * pV;
                if (p <= 1.0 && E <= 5'000'000) {
                    inputs.push_back({U, V, pV});
                }
            }
        }
    }

    const auto runtime = 360'000ms / inputs.size();
    map<tuple<pair<int, int>, int, string>, string> table;

    for (auto [U, V, pV] : inputs) {
        START_ACC2(mm, hop);
        int E = U * pV;

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 1000, runs) {
            print_time(now, runtime, "speed bipartite matching U={} V={} E={} ({} runs)",
                       U, V, E, runs);

            auto g = random_exact_bipartite(U, V, E);
            shuffle(begin(g), end(g), mt);

            ADD_TIME_BLOCK(mm) {
                bipartite_matching mm(U, V);
                for (auto [u, v] : g)
                    mm.add(u, v);
                mm.max_matching();
            }

            ADD_TIME_BLOCK(hop) {
                hopcroft_karp hk(U, V);
                for (auto [u, v] : g)
                    hk.add(u, v);
                hk.max_matching();
            }
        }

        table[{{V, pV}, U, "mm"}] = FORMAT_EACH(mm, runs);
        table[{{V, pV}, U, "hop"}] = FORMAT_EACH(hop, runs);
    }

    print_time_table(table, "Bipartite matching");
}

int main() {
    RUN_BLOCK(stress_test_bipartite_matching());
    RUN_BLOCK(speed_test_bipartite_matching());
    return 0;
}
