#pragma once

#include "random.hpp"

using edges_t = vector<array<int, 2>>;

// Generate a solvable 2sat problem with N variables, E edges.
// Each variable in the canonical solution is true with probability true_p
// Both variables in each edge are true with probability both_p
edges_t generate_twosat(int N, int E, double true_p = 0.5, double both_p = 0.1) {
    intd distv(0, N - 1);
    vector<int> hidden_solution(N);
    for (auto& v : hidden_solution) {
        v = cointoss(true_p);
    }
    edges_t g(E);
    for (auto& [u, v] : g) {
        u = rand_unif<int>(0, N - 1);
        v = rand_unif<int>(0, N - 1);
        u = hidden_solution[u] ? u : ~u; // 'left' node is true
        if (cointoss(both_p)) {
            v = hidden_solution[v] ? v : ~v;
        } else {
            v = hidden_solution[v] ? ~v : v;
        }
        if (cointoss(0.5)) {
            swap(u, v);
        }
    }
    return g;
}

bool verify_twosat(const edges_t& g, const vector<int>& assignment) {
    return all_of(begin(g), end(g), [&](auto edge) {
        auto [u, v] = edge;
        return (u >= 0 && assignment[u]) || (u < 0 && !assignment[~u]) ||
               (v >= 0 && assignment[v]) || (v < 0 && !assignment[~v]);
    });
}
