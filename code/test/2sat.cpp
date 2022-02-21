#include "test_utils.hpp"
#include "algo/2sat.hpp"
#include "lib/2sat.hpp"

void speed_test_twosat_positive() {
    vector<int> Ns = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};
    vector<int> Es = {2, 5, 10, 20, 35};

    vector<pair<int, int>> inputs;
    for (int N : Ns) {
        for (int E : Es) {
            if (N * E <= 5'000'000) {
                inputs.push_back({N, E});
            }
        }
    }
    const auto runtime = 90'000ms / inputs.size();
    map<pair<int, int>, string> table;

    for (auto [N, E] : inputs) {
        START_ACC(sat);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 1000, runs) {
            print_time(now, runtime, "speed 2-sat N,E={},{}", N, N * E);

            auto g = generate_twosat(N, N * E);

            START(sat);
            twosat_scc sat(N);
            for (auto [u, v] : g) {
                sat.either(u, v);
            }
            bool ok = sat.solve();
            ADD_TIME(sat);

            assert(ok && verify_twosat(g, sat.assignment));
        }

        table[{E, N}] = FORMAT_EACH(sat, runs);
    }

    print_time_table(table, "2SAT");
}

int main() {
    RUN_BLOCK(speed_test_twosat_positive());
    return 0;
}
