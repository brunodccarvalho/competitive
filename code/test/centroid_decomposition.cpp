#include "test_utils.hpp"
#include "graphs/centroid_decomposition.hpp"
#include "lib/graph_generator.hpp"
#include "lib/graph_formats.hpp"

void unit_test_centroid_decomposition() {
    int V = 38;
    string sedges = "0,1 0,2 0,3 0,4 1,5 1,6 2,7 2,8 3,9 3,10 6,11 6,36 6,37 7,28 7,29 "
                    "8,12 8,32 8,33 9,14 9,15 9,20 10,13 10,16 10,17 11,34 11,35 12,30 "
                    "12,31 13,18 13,19 14,24 14,25 14,26 14,27 15,21 15,23 21,22";

    edges_t g = scan_edges(sedges);
    random_flip_graph_inplace(g);
    assert(int(g.size()) == V - 1);

    auto tree = make_adjacency_lists_undirected(V, g);
    auto [cparent, cdepth] = build_centroid_decomposition(tree);
    for (int u = 0; u < V; u++) {
        cout << (u ? " " : "        ") << setw(2) << u << " \n"[u + 1 == V];
    }
    for (int u = 0; u < V; u++) {
        cout << (u ? " " : "cparent ") << setw(2) << cparent[u] << " \n"[u + 1 == V];
    }
    for (int u = 0; u < V; u++) {
        cout << (u ? " " : "cdepth  ") << setw(2) << cdepth[u] << " \n"[u + 1 == V];
    }
}

int main() {
    RUN_BLOCK(unit_test_centroid_decomposition());
    return 0;
}
