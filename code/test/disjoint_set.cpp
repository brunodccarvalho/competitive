#include "test_utils.hpp"
#include "struct/disjoint_set.hpp"
#include "algo/y_combinator.hpp"

bool is_bipartite(int N, const vector<array<int, 2>>& graph) {
    vector<vector<int>> adj(N);
    for (auto [u, v] : graph) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    vector<int> color(N, -1);
    auto dfs = y_combinator([&](auto self, int u) -> bool {
        bool ok = true;
        for (int v : adj[u]) {
            if (color[v] == -1) {
                color[v] = 1 - color[u];
                ok &= self(v);
            } else {
                ok &= color[u] != color[v];
            }
        }
        return ok;
    });
    bool ok = true;
    for (int u = 0; u < N && ok; u++) {
        if (color[u] == -1) {
            color[u] = 0;
            ok &= dfs(u);
        }
    }
    return ok;
}

void unit_test_dsu_rollback() {
    disjoint_set_rollback dsu(10);

    dsu.join(1, 2);
    dsu.join(0, 3);
    // (0 3) (1 2) 4 5 6 7 8 9
    int t0 = dsu.time();
    dsu.join(4, 6);
    dsu.join(1, 9);
    dsu.join(0, 5);
    dsu.join(0, 6);
    // (0 3 4 5 6) (1 2 9) 7 8
    int t1 = dsu.time();
    dsu.join(8, 1);
    dsu.join(3, 9);
    dsu.join(2, 4);
    // (0 3 4 5 6 1 2 8 9) 7

    assert(dsu.same(2, 0));
    assert(!dsu.same(1, 7));
    assert(dsu.size(dsu.find(0)) == 9 && dsu.size(7) == 1);

    dsu.rollback(t1);

    assert(!dsu.same(2, 0));
    assert(dsu.same(0, 4));
    assert(dsu.size(dsu.find(0)) == 5 && dsu.size(dsu.find(1)) == 3);

    dsu.rollback(t0);

    assert(!dsu.same(2, 0));
    assert(!dsu.same(0, 4));
    assert(dsu.same(1, 2));
    assert(dsu.size(dsu.find(0)) == 2 && dsu.size(dsu.find(2)) == 2);
}

void stress_test_bipartite_dsu() {
    mt.seed(73);
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (10s, now, 1'000'000, runs) {
        print_time(now, 10s, "stress test bipartite dsu ({} runs)", runs);

        int N = rand_unif<int>(5, 50);
        bipartite_disjoint_set_rollback dsu(N);
        vector<array<int, 2>> graph;
        set<array<int, 2>> edges;
        int R = 100;

        vector<int> times;

        while (R > 0) {
            auto [u, v] = diff_unif<int>(0, N - 1);
            if (!edges.insert({u, v}).second) {
                continue;
            }
            times.push_back(dsu.time());
            graph.push_back({u, v});
            dsu.join(u, v);
            if (is_bipartite(N, graph)) {
                assert(dsu.bipartite());
                assert(dsu.bipartite(u));
                assert(dsu.bipartite(v));
            } else {
                assert(!dsu.bipartite());
                assert(!dsu.bipartite(u));
                assert(!dsu.bipartite(v));
                dsu.rollback(times.back());
                edges.erase(graph.back());
                times.pop_back();
                graph.pop_back();
                R--;
            }
            if (cointoss(0.5)) {
                dsu.rollback(times.back());
                edges.erase(graph.back());
                times.pop_back();
                graph.pop_back();
            }
        }
    }
}

int main() {
    RUN_BLOCK(unit_test_dsu_rollback());
    RUN_BLOCK(stress_test_bipartite_dsu());
    return 0;
}
