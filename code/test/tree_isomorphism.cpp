#include "test_utils.hpp"
#include "graphs/tree_isomorphism.hpp"
#include "lib/graph_operations.hpp"
#include "lib/graph_generator.hpp"

auto relabel_nonroot_inplace(int V, edges_t& g) {
    vector<int> label(V);
    iota(begin(label), end(label), 0);
    shuffle(begin(label) + 1, end(label), mt);
    for (auto& [u, v] : g) {
        u = label[u], v = label[v];
    }
}

void stress_test_unrooted_tree_isomorphism() {
    const int V = 10000;
    int errors = 0;

    LOOP_FOR_DURATION_TRACKED (3s, now) {
        print_time(now, 3s, "stress test unrooted tree isomorphism");

        auto g = random_tree(V);
        random_relabel_graph_inplace(V, g);
        random_flip_graph_inplace(g);
        auto ghash = hash_unrooted_tree(V, g);
        auto gvhash = hash_unrooted_tree_vertices(V, g);
        sort(begin(gvhash), end(gvhash));

        LOOP_FOR_DURATION (50ms) {
            auto h = g;
            random_relabel_graph_inplace(V, h);
            random_flip_graph_inplace(h);
            auto hhash = hash_unrooted_tree(V, h);
            auto hvhash = hash_unrooted_tree_vertices(V, h);
            sort(begin(hvhash), end(hvhash));
            errors += ghash != hhash;
            errors += gvhash != hvhash;
        }
    }

    printcl("ERRORS: {}\n", errors);
}

void stress_test_rooted_tree_isomorphism() {
    const int V = 10000;
    int errors = 0;

    LOOP_FOR_DURATION_TRACKED (3s, now) {
        print_time(now, 3s, "stress test rooted tree isomorphism");

        auto g = random_tree(V);
        relabel_nonroot_inplace(V, g);
        random_flip_graph_inplace(g);
        auto ghash = hash_rooted_tree(V, g, 0);
        auto gvhash = hash_rooted_tree_vertices(V, g, 0);
        sort(begin(gvhash), end(gvhash));

        LOOP_FOR_DURATION (50ms) {
            auto h = g;
            relabel_nonroot_inplace(V, h);
            random_flip_graph_inplace(h);
            auto hhash = hash_rooted_tree(V, h, 0);
            auto hvhash = hash_rooted_tree_vertices(V, h, 0);
            sort(begin(hvhash), end(hvhash));
            errors += ghash != hhash;
            errors += gvhash != hvhash;
        }
    }

    printcl("ERRORS: {}\n", errors);
}

int main() {
    RUN_BLOCK(stress_test_unrooted_tree_isomorphism());
    RUN_BLOCK(stress_test_rooted_tree_isomorphism());
    return 0;
}
