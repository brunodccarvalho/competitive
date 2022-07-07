#include "test_utils.hpp"
#include "graphs/tree_isomorphism.hpp"
#include "struct/disjoint_set.hpp"
#include "lib/graph_operations.hpp"
#include "lib/graph_generator.hpp"

void stress_test_isomorphic_mapping() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (10s, now, 30'000, runs) {
        print_time(now, 10s, "stress test isomorphic mapping ({} runs)", runs);

        const int N = rand_unif<int>(10, 100);
        const int T = rand_wide<int>(1, N, -3);
        auto A = random_forest(N, T);
        auto B = A;
        random_relabel_graph_inplace(N, A);
        random_relabel_graph_inplace(N, B);
        random_flip_graph_inplace(A);
        random_flip_graph_inplace(B);
        shuffle(begin(A), end(A), mt);
        shuffle(begin(B), end(B), mt);

        auto pi = unrooted_topology::isomorphic_mapping(N, A, B);
        assert(pi.size());

        set<array<int, 2>> edges;
        for (auto [u, v] : B) {
            edges.insert({u, v});
            edges.insert({v, u});
        }
        for (auto [u, v] : A) {
            u = pi[u], v = pi[v];
            int e1 = edges.erase({u, v});
            int e2 = edges.erase({v, u});
            assert(e1 && e2);
        }
    }
}

void unit_test_build_branches() {
    const int N = 40, B = 8;
    auto ans = build_branches(N, B, true, false);
    vector<int> hashes;
    for (int V = 1; V <= N; V++) {
        for (int b = 0; b <= B; b++) {
            for (int c = 0; c < V; c++) {
                for (auto G : ans[{V, b, c}]) {
                    hashes.push_back(unrooted_topology::hash_tree(V, G));
                }
            }
        }
    }
    sort(begin(hashes), end(hashes));
    int H1 = hashes.size();
    hashes.erase(unique(begin(hashes), end(hashes)), end(hashes));
    int H2 = hashes.size();
    putln(H1, H2);
}

void unit_test_build_forks() {
    const int N = 25, B = 3;
    auto ans = build_branches(N, B, false, true);
    vector<int> hashes;
    for (int V = 1; V <= N; V++) {
        for (int b = 0; b <= B; b++) {
            for (int c = 0; c < V; c++) {
                for (auto G : ans[{V, b, c}]) {
                    hashes.push_back(unrooted_topology::hash_tree(V, G));
                }
            }
        }
    }
    sort(begin(hashes), end(hashes));
    int H1 = hashes.size();
    hashes.erase(unique(begin(hashes), end(hashes)), end(hashes));
    int H2 = hashes.size();
    putln(H1, H2);
}

void unit_test_unrooted_binaries() {
    const int N = 20;
    auto ans = build_unrooted_binary_trees(N);
    vector<int> hashes, counts;
    for (int v = 0; v <= N; v++) {
        counts.push_back(ans[v].size());
        for (const auto& G : ans[v]) {
            int V = G.size() + 1;
            hashes.push_back(unrooted_topology::hash_tree(V, G));
        }
    }
    putln(counts);
    sort(begin(hashes), end(hashes));
    int H1 = hashes.size();
    hashes.erase(unique(begin(hashes), end(hashes)), end(hashes));
    int H2 = hashes.size();
    assert(H1 == H2);
}

void unit_test_bounded_trees() {
    const int N = 15;
    auto ans = build_bounded_degree(N, 5);
    vector<int> hashes, counts;
    for (int V = 1; V <= N; V++) {
        counts.push_back(ans[V].size());
        for (const auto& G : ans[V]) {
            hashes.push_back(unrooted_topology::hash_tree(V, G));
        }
    }
    putln(counts);
    sort(begin(hashes), end(hashes));
    int H1 = hashes.size();
    hashes.erase(unique(begin(hashes), end(hashes)), end(hashes));
    int H2 = hashes.size();
    assert(H1 == H2);
}

void speed_test_visit_subtrees() {
    for (int N = 20; N <= 40; N += 4) {
        unrooted_topology::clear();
        START_ACC(dfs);
        LOOP_FOR_DURATION_OR_RUNS_TRACKED (30s, now, 100, runs) {
            auto tree = random_tree(N);
            random_relabel_graph_inplace(N, tree);
            random_flip_graph_inplace(tree);
            shuffle(begin(tree), end(tree), mt);
            int64_t S = 0, T = 0;
            auto vis = [&](int V, int, const auto&, const auto&) { S += V, T += 1; };
            ADD_TIME_BLOCK(dfs) { unrooted_topology::visit_subtrees(N, tree, vis); }
        }
        println("{:>20} N={} | {} runs", FORMAT_EACH(dfs, runs), N, runs);
    }
}

int main() {
    RUN_BLOCK(stress_test_isomorphic_mapping());
    RUN_BLOCK(unit_test_build_branches());
    RUN_BLOCK(unit_test_build_forks());
    RUN_BLOCK(unit_test_unrooted_binaries());
    RUN_BLOCK(unit_test_bounded_trees());
    RUN_BLOCK(speed_test_visit_subtrees());
    return 0;
}
