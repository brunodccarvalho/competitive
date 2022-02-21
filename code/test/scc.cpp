#include "test_utils.hpp"
#include "lib/graph_formats.hpp"
#include "graphs/scc.hpp"

void unit_test_scc() {
    // vertex 0 is completely disconnected
    const int V = 9;
    edges_t g = scan_edges("1,2 2,3 3,1 4,2 4,3 4,6 5,3 5,7 6,4 6,5 7,5 8,6 8,7 8,8");
    auto adj = make_adjacency_lists_directed(V, g);
    auto [C, cmap] = build_scc(adj);
    auto [sccedges, sccout, sccin] = condensate_scc(C, adj, cmap);

    assert(C == 5); // num components
    for (int c = 0; c < C; c++) {
        sort(begin(sccout[c]), end(sccout[c]));
        sort(begin(sccin[c]), end(sccin[c]));
    }

    using vi = vector<int>;

    assert(sccout[0] == vi() && sccin[0] == vi());
    assert(sccout[1] == vi() && sccin[1] == vi({2, 3}));
    assert(sccout[2] == vi({1}) && sccin[2] == vi({3, 4}));
    assert(sccout[3] == vi({1, 2}) && sccin[3] == vi({4}));
    assert(sccout[4] == vi({2, 3}) && sccin[4] == vi());
}

int main() {
    RUN_BLOCK(unit_test_scc());
    return 0;
}
