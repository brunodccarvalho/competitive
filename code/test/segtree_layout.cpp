#include "test_utils.hpp"
#include "struct/segtree_layout.hpp"

void stress_test_layout() {
    for (int N : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 16, 17, 127, 128, 129, 130}) {
        standard_layout layout(N);
        for (int i = 0; i < N; i++) {
            auto u = layout.point(i);
            assert(layout.get_leaf_index(u) == i);
            assert(layout.get_node_bounds(u) == standard_layout::interval({i, i + 1}));
            assert(layout.get_node_size(u) == 1);
        }
        for (segpoint a = N - 1; a >= 1; a--) {
            assert(layout.get_node_size(a) ==
                   layout.get_node_size(a[0]) + layout.get_node_size(a[1]));
            assert(layout.get_node_bounds(a)[0] == layout.get_node_bounds(a[0])[0]);
            assert(layout.get_node_bounds(a)[1] == layout.get_node_bounds(a[1])[1]);

            int m = layout.get_node_split(a);
            assert(m == layout.get_node_bounds(a[0])[1]);
            assert(m == layout.get_node_bounds(a[1])[0]);
        }

        vector<int> arr(N, 1);

        for (int l = 0; l <= N; l++) {
            for (int r = l; r <= N; r++) {
                auto ab = layout.range(l, r);

                { // We go inwards
                    int x = l, y = r;
                    ab.for_each([&](auto a) {
                        auto bounds = layout.get_node_bounds(a);
                        if (x == bounds[0]) {
                            x = bounds[1];
                        } else if (y == bounds[1]) {
                            y = bounds[0];
                        } else {
                            assert(false);
                        }
                    });
                    assert(x == y);
                }
                { // We go inwards
                    int x = l, y = r;
                    ab.for_each_with_side([&](auto a, bool d) {
                        auto bounds = layout.get_node_bounds(a);
                        if (d == 0) {
                            assert(x == bounds[0]);
                            x = bounds[1];
                        } else if (d == 1) {
                            assert(y == bounds[1]);
                            y = bounds[0];
                        } else
                            assert(false);
                    });
                    assert(x == y);
                }
                { // We go rightwards
                    int x = l;
                    ab.for_each_l_to_r([&](auto a) {
                        auto bounds = layout.get_node_bounds(a);
                        assert(x == bounds[0]);
                        x = bounds[1];
                    });
                    assert(x == r);
                }
                { // We go leftwards
                    int y = r;
                    ab.for_each_r_to_l([&](auto a) {
                        auto bounds = layout.get_node_bounds(a);
                        assert(y == bounds[1]);
                        y = bounds[0];
                    });
                    assert(y == l);
                }

                // But how do you add ranged binary search exactly?
                // You just do it recursively really... oh well
            }
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_layout());
    return 0;
}
