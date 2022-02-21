#include "test_utils.hpp"
#include "struct/centroid_forest.hpp"
#include "struct/shallow_forest.hpp"
#include "lib/graph_generator.hpp"
#include "lib/graph_formats.hpp"

void speed_test_centroid_forest() {
    vector<int> Ns = {10'000, 40'000, 100'000, 300'000, 600'000, 1'000'000};
    vector<int> Rs = {10'000, 40'000, 100'000, 300'000, 600'000, 1'000'000};

    vector<pair<int, int>> inputs;
    for (int N : Ns) {
        for (int R : Rs) {
            inputs.emplace_back(N, R);
        }
    }
    map<pair<pair<int, int>, string>, stringable> table;

    for (auto [N, R] : inputs) {
        printcl("speed centroid/shallow forest N={} R={}", N, R);
        START_ACC3(ctd, ctd_build, ctd_query);
        START_ACC3(shw, shw_build, shw_query);

        auto g = random_geometric_tree(N, .02);
        vector<vector<int>> tree(N);
        for (auto [u, v] : g) {
            tree[u].push_back(v), tree[v].push_back(u);
        }

        vector<long> arr = rands_unif<long>(N, -500, 500);
        vector<array<int, 2>> queries(R);
        vector<long> a(R), b(R);

        for (int i = 0; i < R; i++) {
            queries[i] = different<int>(0, N - 1);
        }

        ADD_TIME_BLOCK(ctd) {
            START(ctd_build);
            centroid_disjoint_sparse_table ctd(tree, arr, plus<long>{});
            ADD_TIME(ctd_build);

            START(ctd_query);
            for (int i = 0; i < R; i++) {
                auto [u, v] = queries[i];
                a[i] = ctd.query(u, v);
            }
            ADD_TIME(ctd_query);
        }

        ADD_TIME_BLOCK(shw) {
            START(shw_build);
            shallow_disjoint_sparse_table shw(tree, arr, plus<long>{});
            ADD_TIME(shw_build);

            START(shw_query);
            for (int i = 0; i < R; i++) {
                auto [u, v] = queries[i];
                b[i] = shw.query(u, v);
            }
            ADD_TIME(shw_query);
        }

        // Same responses to the queries
        assert(a == b);

        table[{{N, R}, "ctd"}] = FORMAT_TIME(ctd);
        table[{{N, R}, "ctd_build"}] = FORMAT_TIME(ctd_build);
        table[{{N, R}, "ctd_query"}] = FORMAT_TIME(ctd_query);
        table[{{N, R}, "shw"}] = FORMAT_TIME(shw);
        table[{{N, R}, "shw_build"}] = FORMAT_TIME(shw_build);
        table[{{N, R}, "shw_query"}] = FORMAT_TIME(shw_query);
    }

    print_time_table(table, "Centroid vs shallow forest");
}

int main() {
    RUN_BLOCK(speed_test_centroid_forest());
    return 0;
}
