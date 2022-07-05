#include "test_utils.hpp"
#include "graphs/tree_isomorphism.hpp"
#include "lib/graph_operations.hpp"
#include "lib/graph_generator.hpp"

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
    const int N = 30, B = 3;
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

int main() {
    RUN_BLOCK(unit_test_build_branches());
    RUN_BLOCK(unit_test_build_forks());
    RUN_BLOCK(unit_test_unrooted_binaries());
    return 0;
}
