#include "test_utils.hpp"
#include "algo/line_tree.hpp"
#include "algo/virtual_tree.hpp"
#include "algo/y_combinator.hpp"
#include "struct/shallow_forest.hpp"
#include "struct/lca.hpp"
#include "struct/sparse_table.hpp"
#include "lib/graph_generator.hpp"

auto stress_test_line_tree() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (3s, now, 10000, runs) {
        print_time(now, 3s, "stress line tree ({} runs)", runs);

        int N = rand_unif<int>(2, 1000);
        auto g = random_tree(N);
        auto weight = int_sample<int>(N - 1, 1, 1'000'000);
        auto tree = make_adjacency_lists_undirected(N, g);

        vector<int> pointval(N, INT_MAX);
        vector<vector<pair<int, int>>> val_tree(N);
        for (int e = 0; e < N - 1; e++) {
            auto [u, v] = g[e];
            val_tree[u].push_back({v, weight[e]});
            val_tree[v].push_back({u, weight[e]});
        }

        auto minop = [](int a, int b) { return min(a, b); };
        shallow_edge_sparse_table shallow(tree, pointval, val_tree, minop);

        auto [index, edge, value] = build_min_edge_line(N, g, weight);
        min_rmq<int> rmq1(value);

        auto [root, parent, kids, intime, _] = build_min_edge_virtual_tree(N, g, weight);
        vector<vector<int>> virtual_tree(2 * N - 1);
        for (int e = 0; e < N - 1; e++) {
            int y = N + e, u = kids[y][0], v = kids[y][1];
            virtual_tree[y].push_back(u), virtual_tree[u].push_back(y);
            virtual_tree[y].push_back(v), virtual_tree[v].push_back(y);
        }
        lca_incremental lca(virtual_tree, root);

        for (int u = 0; u < N; u++) {
            for (int v = 0; v < N; v++) {
                if (u != v) {
                    int a = min(index[u], index[v]);
                    int b = max(index[u], index[v]);
                    assert(rmq1.query(a, b) == shallow.query(u, v));
                    assert(weight[lca.lca(u, v) - N] == shallow.query(u, v));
                }
            }
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_line_tree());
    return 0;
}
