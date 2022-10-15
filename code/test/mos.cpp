#include "test_utils.hpp"
#include "algo/mos.hpp"
#include "random.hpp"

void stress_test_mos() {
    vector<pair<int, int>> NQs = {
        {100, 100},       //
        {100, 500},       //
        {1000, 1000},     //
        {10000, 10000},   //
        {100000, 1000},   //
        {100000, 30000},  //
        {100000, 100000}, //
        {100000, 300000}, //
    };

    auto compute = [](const auto& queries, const auto& order) {
        int Q = queries.size();
        long init = queries[order[0]][1] - queries[order[0]][0];
        long shifts = init;
        long jumps = init;
        long transfers = init;
        for (int i = 1; i < Q; i++) {
            auto [L, R] = queries[order[i - 1]];
            auto [l, r] = queries[order[i]];
            shifts += abs(L - l) + abs(R - r);
            jumps += min(r - l, abs(L - l) + abs(R - r));
            transfers += min(abs(L - l) + abs(R - r), abs(R - L) + abs(r - l));
        }
        return make_tuple(shifts, jumps, transfers);
    };

    map<pair<string, pair<int, int>>, stringable> table;

    for (auto [N, Q] : NQs) {
        vector<array<int, 2>> queries(Q);
        for (auto& [l, r] : queries) {
            auto [u, v] = diff_unif<int>(0, N - 1);
            l = u, r = v;
        }

        auto left_block_order = mosort_left_block(N, queries);
        auto hilbert_order = mosort_hilbert_curve(N, queries);

        auto [lb_shifts, lb_jumps, lb_transfers] = compute(queries, left_block_order);
        auto [hi_shifts, hi_jumps, hi_transfers] = compute(queries, hilbert_order);

        table[{"block shift", {N, Q}}] = lb_shifts;
        table[{"block shift/jump", {N, Q}}] = lb_jumps;
        table[{"block shift/transfer", {N, Q}}] = lb_transfers;
        table[{"hilbert shift", {N, Q}}] = hi_shifts;
        table[{"hilbert shift/jump", {N, Q}}] = hi_jumps;
        table[{"hilbert shift/transfer", {N, Q}}] = hi_transfers;
    }

    print_time_table(table, "Mo's updates");
}

int main() {
    RUN_SHORT(stress_test_mos());
    return 0;
}
