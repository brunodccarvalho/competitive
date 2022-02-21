#include "test_utils.hpp"
#include "struct/link_cut_tree_path.hpp"
#include "struct/link_cut_tree_edge_path.hpp"

using lct_edge_path = link_cut_tree_edge_path<lct_node_path_max<long>>;

void unit_test_lct_edge_path() {
    const int N = 10;
    lct_edge_path lct(N);

    bool ok = true;
    ok &= lct.link(1, 2, 7);
    ok &= lct.link(2, 4, 2);
    ok &= lct.link(3, 7, 2);
    ok &= lct.link(1, 5, 11);
    ok &= lct.link(1, 3, 5);
    ok &= lct.link(8, 10, 6);
    ok &= lct.link(3, 8, 4);
    ok &= lct.link(3, 6, 8);
    ok &= lct.link(6, 9, 9);
    assert(ok);

    lct.reroot(2);
    cout << lct.lca(3, 5) << '\n'; // 1
    lct.reroot(3);
    cout << lct.lca(4, 5) << '\n'; // 1
    lct.reroot(7);
    cout << lct.lca(9, 4) << '\n'; // 3
    lct.reroot(2);
    cout << lct.lca(3, 5) << '\n'; // 1

    cout << lct.access_path(2, 7)->path << '\n'; // 7
    cout << lct.access_path(6, 5)->path << '\n'; // 11
    cout << lct.access_path(8, 9)->path << '\n'; // 9
}

int main() {
    RUN_SHORT(unit_test_lct_edge_path());
    return 0;
}
