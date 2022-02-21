#include "test_utils.hpp"
#include "graphs/travelling_salesman.hpp"
#include "heuristic/tsp_ant_colony.hpp"

void unit_test_stochastic_tsp() {
    int n = 1000, C = 2'000'000;

    // n=1000,repulsion=0:   90s,48s,35s
    // n=1000,repulsion=-1:  60s <4/<8
    // n=1000,repulsion=-3: 120s <6/<12, can still continue making progress

    vector<vector<long>> cost(n, vector<long>(n));
    long sum_all_costs = 0;

    for (int u = 0; u < n; u++) {
        for (int v = 0; v < n; v++) {
            if (u != v) {
                cost[u][v] = rand_wide<int>(1, C, -1);
                sum_all_costs += cost[u][v];
            }
        }
    }

    long opt_cost = 0;
    for (int i = 0; i < n; i++) {
        int u = i, v = (i + 1) % n;
        sum_all_costs -= cost[u][v];
        cost[u][v] = rand_wide<int>(1, C / n / 5, -3);
        sum_all_costs += cost[u][v];
        opt_cost += cost[u][v];
    }

    println("average cost: {}", sum_all_costs / (n - 1));
    println(" opt cost: {}", opt_cost);

    auto [ants_cost, ants] = ant_colony_tour(cost, 5min);
    println("ants cost: {}", ants_cost);
    println("ants: {}", ants);
}

int main() {
    RUN_BLOCK(unit_test_stochastic_tsp());
    return 0;
}
