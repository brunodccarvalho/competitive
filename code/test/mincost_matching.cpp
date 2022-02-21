#include "test_utils.hpp"
#include "lib/bipartite_matching.hpp"
#include "matching/fast_dense_hungarian.hpp"

void stress_test_mincost_matching() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 50000, runs) {
        print_time(now, 20s, "stress mincost matching ({} runs)", runs);

        int V = rand_unif<int>(1, 200);

        vector<vector<int>> cost(V, vector<int>(V));
        for (int u = 0; u < V; u++) {
            for (int v = 0; v < V; v++) {
                cost[u][v] = u == v ? 0 : rand_unif<int>(1, 50'000'000);
            }
        }

        fast_dense_hungarian<int, int64_t>(cost);

        // TODO: augmenting path verification
    }
}

void speed_test_mincost_matching() {
    vector<int> Vs = {60, 150, 300, 500, 1000, 2000, 3000, 5000, 8000};

    vector<int> inputs = Vs;

    const auto runtime = 120'000ms / inputs.size();
    map<pair<string, int>, stringable> table;

    for (int V : inputs) {
        START_ACC(dense);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 1000, runs) {
            print_time(now, runtime, "speed mincost matching V={} ({} runs)", V, runs);

            vector<vector<int>> cost(V, vector<int>(V));
            for (int u = 0; u < V; u++) {
                for (int v = 0; v < V; v++) {
                    cost[u][v] = u == v ? 0 : rand_unif<int>(1, 50'000'000);
                }
            }

            ADD_TIME_BLOCK(dense) { fast_dense_hungarian<int, int64_t>(cost); }
        }

        table[{"hungarian", V}] = FORMAT_EACH(dense, runs);
    }

    print_time_table(table, "Mincost bipartite matching");
}

int main() {
    RUN_BLOCK(stress_test_mincost_matching());
    RUN_BLOCK(speed_test_mincost_matching());
    return 0;
}
