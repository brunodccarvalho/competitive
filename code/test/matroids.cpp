#include "test_utils.hpp"
#include "linear/matroids/matroid_intersection.hpp"
#include "linear/matroids/matroid_partitioning.hpp"
#include "linear/matroids/matroid_generator.hpp"
#include "lib/graph_generator.hpp"
#include "lib/graph_formats.hpp"
#include "algo/y_combinator.hpp"
#include "numeric/bits.hpp"

// O(n (n choose k))
template <typename Matroid1, typename Matroid2>
auto naive_max_matroid_intersection(Matroid1 m1, Matroid2 m2) {
    assert(m1.ground_size() == m2.ground_size());
    m1.clear(), m2.clear();

    int n = m1.ground_size();
    int A = 0;
    int B = min(find_basis(m1).size(), find_basis(m2).size());

    y_combinator([&](auto self, int start, int size, auto M1, auto M2) -> void {
        A = max(A, size);
        for (int i = start; i < n && A < B; i++) {
            if (M1.check(i) && M2.check(i)) {
                auto M1_copy = M1;
                auto M2_copy = M2;
                M1_copy.insert(i);
                M2_copy.insert(i);
                self(i + 1, size + 1, M1_copy, M2_copy);
            }
        }
    })(0, 0, m1, m2);

    return A;
}

// O(n (n choose k))
template <typename Matroid1, typename Matroid2, typename Cost>
auto naive_mincost_matroid_intersection(const vector<Cost>& cost, Matroid1 m1,
                                        Matroid2 m2) {
    assert(m1.ground_size() == m2.ground_size());
    assert(*min_element(begin(cost), end(cost)) >= 0);
    m1.clear(), m2.clear();

    int n = m1.ground_size();
    int A = naive_max_matroid_intersection(m1, m2);
    Cost mincost = numeric_limits<Cost>::max();

    y_combinator([&](auto self, Cost far, int start, int size, auto M1, auto M2) -> void {
        if (far >= mincost) {
            return;
        }
        if (size == A) {
            mincost = far;
            return;
        }
        for (int i = start; i < n && size + n - i >= A; i++) {
            if (M1.check(i) && M2.check(i)) {
                auto M1_copy = M1;
                auto M2_copy = M2;
                M1_copy.insert(i);
                M2_copy.insert(i);
                self(far + cost[i], i + 1, size + 1, M1_copy, M2_copy);
            }
        }
    })(0, 0, 0, m1, m2);

    return make_pair(A, mincost);
}

// O(k^n)
template <typename Cost, typename Matroid>
auto naive_mincost_matroid_partitioning(const vector<vector<Cost>>& cost,
                                        vector<Matroid> ms) {
    int k = ms.size();
    int n = ms[0].ground_size();

    for (int s = 0; s < k; s++) {
        assert(*min_element(begin(cost[s]), end(cost[s])) >= 0);
        ms[s].clear();
    }

    pair<int, Cost> best = {0, 0};

    y_combinator([&](auto self, Cost sofar, int start, int size, auto matroids) -> void {
        if (size > best.first) {
            best = {size, sofar};
        } else if (size == best.first && sofar < best.second) {
            best.second = sofar;
        }
        if (best.first == n && sofar >= best.second) {
            return;
        }
        for (int i = start; i < n && size + n - i >= best.first; i++) {
            for (int s = 0; s < k; s++) {
                if (matroids[s].check(i)) {
                    auto copies = matroids;
                    copies[s].insert(i);
                    self(sofar + cost[s][i], i + 1, size + 1, copies);
                }
            }
        }
    })(0, 0, 0, ms);

    return best;
}

// Verify a matroid oracle is indeed a matroid. O(2^n)
template <typename Matroid>
auto matroid_verify(Matroid m) {
    int n = m.ground_size(), B = 0;
    assert(n <= 20);
    // Compute greedy basis size
    for (int i = 0; i < n; i++) {
        B += m.insert_check(i);
    }
    if (B == 0) {
        println("ERROR: Empty matroid");
        return false;
    }
    // Find which sets are independent
    vector<int8_t> independent(1 << n, true);
    for (int mask = 1; mask < (1 << n); mask++) {
        m.clear();
        for (int i = 0; i < n && independent[mask]; i++) {
            independent[mask] &= !(mask >> i & 1) || m.insert_check(i);
        }
        // Verify no set of size >B is independent
        if (__builtin_popcount(mask) > B && independent[mask]) {
            println("ERROR: Set {:B} is independent but B={}", mask, B);
            return false;
        }
    }
    // Verify all sets of size <B which are independent can be extended
    for (int mask = 1; mask < (1 << n); mask++) {
        if (__builtin_popcount(mask) < B && independent[mask]) {
            bool found = false;
            for (int i = 0; i < n && !found; i++) {
                found |= !(mask >> i & 1) && independent[mask | (1 << i)];
            }
            if (!found) {
                println("ERROR: Set {:B} cannot be extended for B={}", mask, B);
                return false;
            }
        }
    }
    // Verify all subsets of independent sets are independent
    for (int mask = 1; mask < (1 << n); mask++) {
        if (independent[mask]) {
            for (int i = 0; i < n; i++) {
                int rem = mask ^ (1 << i);
                if ((mask >> i & 1) && !independent[rem]) {
                    println("ERROR: Set {:B} is independent but {:B} is not", mask, rem);
                    return false;
                }
            }
        }
    }
    return true;
}

void stress_test_matroid_oracles() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (30s, now, 200'000, runs) {
        int n = rand_wide<int>(1, 20, -3);
        auto type = rand_matroid_type();
        auto name = to_string(type);

        print_time(now, 30s, "stress oracles {:2} {:22} ({} runs)", n, name, runs);

        auto matroid = rand_matroid(type, n);
        assert(matroid_verify(matroid));
    }
}

void speed_test_matroid_intersection() {
    vector<int> ns = {15, 30, 50, 100, 200, 300, 500, 800, 1200};
    vector<int> Vs = {8, 20, 50, 120, 300, 750};
    vector<int> Cs = {8, 20, 50, 120, 300, 750};

    vector<tuple<int, int, int>> inputs;
    for (int n : ns) {
        for (int V : Vs) {
            for (int C : Cs) {
                if (V <= n && C <= n && V * (V - 1) / 2 >= n) {
                    inputs.emplace_back(n, V, C);
                }
            }
        }
    }
    const auto runtime = 180'000ms / inputs.size();
    map<tuple<pair<int, int>, int, string>, stringable> table;

    for (auto [n, V, C] : inputs) {
        START_ACC3(colorful_graphic, graphic_colorful, incremental_colorful);

        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            print_time(now, runtime, "speed matroidsect n={} ({} runs)", n, runs);

            colorful_matroid m1 = rand_colorful_matroid(n, C);
            incremental_graphic_matroid m2 = rand_incremental_graphic_matroid(n, V);
            graphic_matroid m3(V, m2.g);

            START(colorful_graphic);
            auto I13 = max_matroid_intersection_11(m1, m3);
            int M13 = I13.size();
            ADD_TIME(colorful_graphic);

            START(graphic_colorful);
            auto I31 = max_matroid_intersection_11(m3, m1);
            int M31 = I31.size();
            ADD_TIME(graphic_colorful);

            START(incremental_colorful);
            auto I21 = max_matroid_intersection_01(m2, m1);
            int M21 = I21.size();
            ADD_TIME(incremental_colorful);

            assert(M13 == M31 && M31 == M21);
        }

        table[{{V, C}, n, "C/G"}] = FORMAT_EACH(colorful_graphic, runs);
        table[{{V, C}, n, "G/C"}] = FORMAT_EACH(graphic_colorful, runs);
        table[{{V, C}, n, "I/C"}] = FORMAT_EACH(incremental_colorful, runs);
    }

    print_time_table(table, "Matroid intersection");
}

void speed_test_mincost_matroid_intersection() {
    vector<int> ns = {15, 30, 50, 100, 200, 300, 500, 800, 1200};
    vector<int> Vs = {8, 20, 50, 120, 300, 750};
    vector<int> Cs = {8, 20, 50, 120, 300, 750};

    vector<tuple<int, int, int>> inputs;
    for (int n : ns) {
        for (int V : Vs) {
            for (int C : Cs) {
                if (V <= n && C <= n && V * (V - 1) / 2 >= n) {
                    inputs.emplace_back(n, V, C);
                }
            }
        }
    }
    const auto runtime = 240'000ms / inputs.size();
    map<tuple<pair<int, int>, int, string>, stringable> table;

    for (auto [n, V, C] : inputs) {
        START_ACC3(colorful_graphic, graphic_colorful, incremental_colorful);

        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            print_time(now, runtime, "speed mincost matroidsect n={} ({} runs)", n, runs);

            auto cost = rands_unif<int>(n, 0, 100'000);

            colorful_matroid m1 = rand_colorful_matroid(n, C);
            incremental_graphic_matroid m2 = rand_incremental_graphic_matroid(n, V);
            graphic_matroid m3(V, m2.g);

            START(colorful_graphic);
            auto [I13, cost13] = mincost_matroid_intersection_11(cost, m1, m3);
            int M13 = I13.size();
            ADD_TIME(colorful_graphic);

            START(graphic_colorful);
            auto [I31, cost31] = mincost_matroid_intersection_11(cost, m3, m1);
            int M31 = I31.size();
            ADD_TIME(graphic_colorful);

            START(incremental_colorful);
            auto [I21, cost12] = mincost_matroid_intersection_01(cost, m2, m1);
            int M21 = I21.size();
            ADD_TIME(incremental_colorful);

            assert(M13 == M31 && M31 == M21);
        }

        table[{{V, C}, n, "C/G"}] = FORMAT_EACH(colorful_graphic, runs);
        table[{{V, C}, n, "G/C"}] = FORMAT_EACH(graphic_colorful, runs);
        table[{{V, C}, n, "I/C"}] = FORMAT_EACH(incremental_colorful, runs);
    }

    print_time_table(table, "Mincost matroid intersection");
}

void speed_test_mincost_matroid_partitioning() {
    vector<int> ns = {15, 30, 50, 100, 200, 300, 500, 800, 1200};
    vector<int> ks = {15, 30, 50, 100, 200, 300, 500, 800};

    vector<pair<int, int>> inputs;
    for (int n : ns) {
        for (int k : ks) {
            if (k <= n) {
                inputs.emplace_back(n, k);
            }
        }
    }
    const auto runtime = 240'000ms / inputs.size();
    map<tuple<int, int, string>, stringable> table;

    for (auto [n, k] : inputs) {
        START_ACC2(spfa, dijkstra);

        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            print_time(now, runtime, "speed mincost matroidpart n={} ({} runs)", n, runs);

            vector<any_matroid_exchange_t> ms;
            vector<vector<int>> cost(k);
            for (int s = 0; s < k; s++) {
                ms.emplace_back(rand_exchange_matroid(n));
                cost[s] = rands_unif<int>(n, 0, 50'000);
            }

            START(dijkstra);
            auto [A, ain, acostsum] = mincost_matroid_partitioning_1(cost, ms);
            ADD_TIME(dijkstra);

            START(spfa);
            auto [B, bin, bcostsum] = mincost_matroid_partitioning_1_spfa(cost, ms);
            ADD_TIME(spfa);

            assert(A == B && acostsum == bcostsum);
        }

        table[{k, n, "spfa"}] = FORMAT_EACH(spfa, runs);
        table[{k, n, "dijk"}] = FORMAT_EACH(dijkstra, runs);
    }

    print_time_table(table, "Mincost matroid partitioning");
}

void stress_test_matroid_intersection() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 100'000, runs) {
        print_time(now, 20s, "stress matroid intersection ({} runs)", runs);

        int n = rand_unif<int>(8, 20);

        auto m1 = rand_exchange_matroid(n);
        auto m2 = rand_exchange_matroid(n);

        auto got = max_matroid_intersection_11(m1, m2);
        int A = naive_max_matroid_intersection(m1, m2);
        int M = got.size();
        assert(A == M);
    }
}

void stress_test_mincost_matroid_intersection() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 100'000, runs) {
        print_time(now, 20s, "mincost matroid intersection ({} runs)", runs);

        int n = rand_unif<int>(8, 15);

        vector<int> cost = rands_unif<int>(n, 0, 5);
        auto m1 = rand_exchange_matroid(n);
        auto m2 = rand_exchange_matroid(n);

        auto [got, gotcost] = mincost_matroid_intersection_11(cost, m1, m2);
        auto [A, anscost] = naive_mincost_matroid_intersection(cost, m1, m2);
        int M = got.size();
        assert(A == M && anscost == gotcost);
    }
}

void stress_test_mincost_matroid_partitioning() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 100'000, runs) {
        print_time(now, 20s, "mincost matroid partitioning ({} runs)", runs);

        int k = rand_unif<int>(2, 4);
        int n = rand_unif<int>(6, 10);

        vector<vector<int>> cost(k);
        vector<any_matroid_exchange_t> ms;
        for (int i = 0; i < k; i++) {
            ms.emplace_back(rand_exchange_matroid(n));
            cost[i] = rands_unif<int>(n, 0, 5);
        }

        auto [M, got, gotcost] = mincost_matroid_partitioning_1(cost, ms);
        auto [S, gots, gotcosts] = mincost_matroid_partitioning_1_spfa(cost, ms);
        auto [A, anscost] = naive_mincost_matroid_partitioning(cost, ms);
        assert(M == S && gotcost == gotcosts);
        assert(A == M && anscost == gotcost);
        assert(A == S && anscost == gotcosts);
    }
}

int main() {
    RUN_BLOCK(stress_test_matroid_oracles());
    RUN_BLOCK(stress_test_matroid_intersection());
    RUN_BLOCK(stress_test_mincost_matroid_intersection());
    RUN_BLOCK(stress_test_mincost_matroid_partitioning());
    RUN_BLOCK(speed_test_matroid_intersection());
    RUN_BLOCK(speed_test_mincost_matroid_intersection());
    RUN_BLOCK(speed_test_mincost_matroid_partitioning());
    return 0;
}
