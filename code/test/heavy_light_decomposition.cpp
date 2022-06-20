#include "test_utils.hpp"
#include "lib/graph_formats.hpp"
#include "lib/graph_generator.hpp"
#include "graphs/heavy_light_decomposition.hpp"
#include "struct/hld_forest.hpp"

void unit_test_heavy_light_decomposition() {
    int V = 38;
    string sedges = "0,1 0,2 0,3 0,4 1,5 1,6 2,7 2,8 3,9 3,10 6,11 6,36 6,37 7,28 7,29 "
                    "8,12 8,32 8,33 9,14 9,15 9,20 10,13 10,16 10,17 11,34 11,35 12,30 "
                    "12,31 13,18 13,19 14,24 14,25 14,26 14,27 15,21 15,23 21,22";

    auto g = scan_edges(sedges);
    random_flip_graph_inplace(g);
    assert(int(g.size()) == V - 1);

    auto tree = make_adjacency_lists_undirected(V, g);
    auto [parent, depth, head, tin, tout] = build_heavy_light_decomposition(tree, 0);
    for (int u = 0; u < V; u++) {
        cout << (u ? " " : "       ") << setw(2) << u << " \n"[u + 1 == V];
    }
    for (int u = 0; u < V; u++) {
        cout << (u ? " " : "parent ") << setw(2) << parent[u] << " \n"[u + 1 == V];
    }
    for (int u = 0; u < V; u++) {
        cout << (u ? " " : " depth ") << setw(2) << depth[u] << " \n"[u + 1 == V];
    }
    for (int u = 0; u < V; u++) {
        cout << (u ? " " : "  head ") << setw(2) << head[u] << " \n"[u + 1 == V];
    }
    for (int u = 0; u < V; u++) {
        cout << (u ? " " : "   tin ") << setw(2) << tin[u] << " \n"[u + 1 == V];
    }
    for (int u = 0; u < V; u++) {
        cout << (u ? " " : "  tout ") << setw(2) << tout[u] << " \n"[u + 1 == V];
    }
}

void unit_test_compress_tree() {
    int N = 21, root = 1;
    string edges = "1,2 2,11 11,13 11,14 11,15 2,16 1,5 1,3 3,17 5,4 5,8 5,7 7,19 8,9 "
                   "9,10 9,12 10,18 10,20 9,0";
    auto graph = scan_edges(edges);
    auto tree = make_adjacency_lists_undirected(N, graph);

    hld_forest hld(tree, root);

    auto print = [&](const vector<int>& nodes) {
        println("compress({}): {}", nodes, hld.compress_tree(nodes));
    };
    print({2, 4, 17});
    print({10, 19, 13});
}

int main() {
    RUN_BLOCK(unit_test_heavy_light_decomposition());
    RUN_BLOCK(unit_test_compress_tree());
    return 0;
}
