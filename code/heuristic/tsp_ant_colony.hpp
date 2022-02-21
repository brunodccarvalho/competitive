#pragma once

#include <bits/stdc++.h>
using namespace std;

struct AntColonyParams {
    // Important parameters:
    double ants = 2; // #ants = this*n. Should be at least >=3 for reasonable results.
    int max_stagnations = 20; // after this many stagnant cycles just give up
    double column_start = 0.01;
    double column_end = 0.03;

    // You can leave the following unchanged, should be ok for most graphs

    // Pheromone exponent over time
    double coercion_start = 0.8;
    double coercion_end = 0.8;

    // Pheromone evaporation factor over time
    double evaporation_start = 0.05;
    double evaporation_end = 0.05;

    // Pheromone deposit exponent over time
    double deposit_start = 5.0;
    double deposit_end = 7.0;

    // Cost exponent over time
    double greed_start = 2.0;
    double greed_end = 5.0;

    // Bounds for min/max pheromone
    double min_phero = 0.25;
    double max_phero = 10.0; // actual max is this*n
    double cancel_ratio = 2.5;
};

/**
 * Ant colony heuristic for directed tour
 * Time: O(V column_factor ants) per run
 *
 * The most important parameter of the algorithm is the number of ants. The more you can
 * afford, the better the results will be.
 */
template <typename Cost>
auto ant_colony_tour(const vector<vector<Cost>>& cost, ms runtime = 2s,
                     AntColonyParams params = AntColonyParams()) {
    int n = cost.size();
    int ants = ceil(n * params.ants);

    // These variables change value as time passes and the solution (hopefully) improves
    const double coercion_ratio = params.coercion_end / params.coercion_start;
    double coercion = params.coercion_start;

    const double evaporation_ratio = params.evaporation_end / params.evaporation_start;
    double evaporation = params.evaporation_start;

    const double deposit_ratio = params.deposit_end / params.deposit_start;
    double deposit = params.deposit_start;

    const double greed_ratio = params.greed_end / params.greed_start;
    double greed = params.greed_start;

    const double column_ratio = params.column_end / params.column_start;
    int columns = (n - 1) * params.column_start;

    auto refresh_params = [&](double reltime) {
        coercion = params.coercion_start * pow(coercion_ratio, reltime);
        evaporation = params.evaporation_start * pow(evaporation_ratio, reltime);
        deposit = params.deposit_start * pow(deposit_ratio, reltime);
        greed = params.greed_start * pow(greed_ratio, reltime);
        columns = (n - 1) * params.column_start * pow(column_ratio, reltime);
    };

    const int max_stagnation_counter = 12;
    int stag_counter = 0, stagnations = 0;

    vector<vector<int>> adj(n);
    for (int u = 0; u < n; u++) {
        for (int v = 0; v < n; v++) {
            if (u != v) {
                adj[u].push_back(v);
            }
        }
    }

    vector<vector<double>> phero(n, vector<double>(n, params.min_phero));
    vector<vector<double>> transition(n, vector<double>(n));
    vector<vector<int>> route(ants, vector<int>(n + 1));
    vector<Cost> route_cost(ants);

    // Add pheromone along the edges of the tour
    auto spread = [&](const vector<int>& tour, double add) {
        for (int i = 0; i < n; i++) {
            int u = tour[i], v = tour[i + 1];
            phero[u][v] += add;
        }
    };

    vector<int> best_tour;
    Cost best_cost = std::numeric_limits<Cost>::max();

    auto startime = chrono::steady_clock::now();
    auto now = startime;
    auto endtime = now + runtime;
    int runs = 1;

    do {
        static mt19937 rng(random_device{}());
        uniform_int_distribution<int> start(0, n - 1);
        using pickd = uniform_real_distribution<double>;

        auto g = [&](int u, int v) {
            return pow(phero[u][v], coercion) / pow(cost[u][v] + 1, greed);
        };

        // We order the edges by transition cost and visit the first |columns| active ones
        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                if (u != v) {
                    transition[u][v] = g(u, v);
                }
            }
            sort(begin(adj[u]), end(adj[u]), [&](int v, int w) -> bool {
                return transition[u][v] > transition[u][w];
            });
        }

        vector<int16_t> active(n);
        vector<int> who(n), good_ants;
        vector<double> pi(n);

        Cost run_cost = std::numeric_limits<Cost>::max();
        int run_ant = -1;

        // Walk the ants, one by one
        for (int ant = 0; ant < ants; ant++) {
            int u = ants >= n && ant < n ? ant : start(rng);
            route[ant][0] = route[ant][n] = u;
            route_cost[ant] = 0;

            fill(begin(active), end(active), true);
            active[u] = false;

            bool cancelled = false;

            for (int i = 1; i < n; i++) {
                int added = 0;
                for (int v : adj[u]) {
                    if (active[v]) {
                        pi[added] = transition[u][v];
                        who[added++] = v;
                    }
                    if (added == columns) {
                        break;
                    }
                }
                for (int a = 1; a < added; a++) {
                    pi[a] += pi[a - 1];
                }

                double sum = pi[added - 1];
                double one = pickd(0, sum)(rng);

                for (int a = 0; true; a++) {
                    if (a == added - 1 || one <= pi[a]) {
                        int v = who[a];
                        active[v] = false;
                        route_cost[ant] += cost[u][v];
                        u = route[ant][i] = v;
                        break;
                    }
                }

                if (run_ant != -1 && route_cost[ant] > params.cancel_ratio * run_cost) {
                    cancelled = true;
                    break;
                }
            }

            if (!cancelled) {
                good_ants.push_back(ant);

                route_cost[ant] += cost[u][route[ant][0]];
                if (run_cost > route_cost[ant]) {
                    run_cost = route_cost[ant];
                    run_ant = ant;
                }
            }
        }

        // We'll spread approx. n pheromone over the edges outgoing from each vertex u
        double weight = pow(run_cost, deposit) / params.ants * n / good_ants.size();

        // Evaporate some of the pheromone
        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                phero[u][v] = (1 - evaporation) * phero[u][v];
            }
        }

        // Deposit new pheromone. Add a factor of n more pheromone on the dominant route.
        for (int ant : good_ants) {
            if (ant != run_ant) {
                spread(route[ant], 2 * weight / pow(route_cost[ant], deposit) / n);
            } else {
                spread(route[ant], weight / pow(route_cost[ant], deposit));
            }
        }

        // Update global best solution
        if (best_cost > run_cost) {
            best_cost = run_cost;
            best_tour = route[run_ant];

            stagnations = stag_counter = 0;
            fprintf(stderr, "[ACT] run %d best=%ld\n", runs, int64_t(best_cost));
        }

        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                phero[u][v] = clamp(phero[u][v], params.min_phero, params.max_phero * n);
            }
        }

        if (++stag_counter > max_stagnation_counter) {
            if (++stagnations == params.max_stagnations) {
                break;
            }
            for (int u = 0; u < n; u++) {
                for (int v = 0; v < n; v++) {
                    phero[u][v] = params.min_phero;
                }
            }
            spread(best_tour, params.max_phero * n);

            stag_counter = 0;
            fprintf(stderr, "[ACT] run %d restart\n", runs);
        }

        now = chrono::steady_clock::now();
        double reltime = 1.0 * (now - startime).count() / (endtime - startime).count();
        refresh_params(reltime);
        runs++;
    } while (now < endtime);

    fprintf(stderr, "[ACT] runs: %d\n", runs);
    return make_pair(best_cost, move(best_tour));
}
