#include "test_utils.hpp"
#include "lib/graph_operations.hpp"
#include "lib/graph_formats.hpp"
#include "lib/graph_generator.hpp"
#include "struct/lca.hpp"

void unit_test_lca_tree() {
    int V = 20;
    string s = "1,2 1,3 1,4 1,5 2,6 2,7 3,8 3,9 3,10 5,11 5,12 5,13 "
               "7,14 10,15 10,16 13,17 13,18 13,19";
    auto g = scan_edges(s);
    random_flip_graph_inplace(g);
    auto tree = make_adjacency_lists_undirected(V, g);

    lca_incremental lca(tree, 1);

    assert(lca.lca(11, 19) == 5);
    assert(lca.lca(9, 15) == 3);
    assert(lca.lca(14, 15) == 1);
    assert(lca.lca(11, 13) == 5);
    assert(lca.depth[8] == 2);
    assert(lca.depth[16] == 3);
    assert(lca.dist(7, 17) == 5);
    assert(lca.dist(6, 8) == 4);
    assert(lca.dist(3, 3) == 0);
    assert(lca.dist(3, 15) == 2);
}

int main() {
    RUN_BLOCK(unit_test_lca_tree());
    return 0;
}
