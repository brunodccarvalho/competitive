#include "test_utils.hpp"
#include "struct/disjoint_set.hpp"

void unit_test_dsu_rollback() {
    disjoint_set_rollback dsu(10);

    dsu.join(1, 2);
    dsu.join(0, 3);
    // (0 3) (1 2) 4 5 6 7 8 9
    int t0 = dsu.time();
    dsu.join(4, 6);
    dsu.join(1, 9);
    dsu.join(0, 5);
    dsu.join(0, 6);
    // (0 3 4 5 6) (1 2 9) 7 8
    int t1 = dsu.time();
    dsu.join(8, 1);
    dsu.join(3, 9);
    dsu.join(2, 4);
    // (0 3 4 5 6 1 2 8 9) 7

    assert(dsu.same(2, 0));
    assert(!dsu.same(1, 7));
    assert(dsu.size(dsu.find(0)) == 9 && dsu.size(7) == 1);

    dsu.rollback(t1);

    assert(!dsu.same(2, 0));
    assert(dsu.same(0, 4));
    assert(dsu.size(dsu.find(0)) == 5 && dsu.size(dsu.find(1)) == 3);

    dsu.rollback(t0);

    assert(!dsu.same(2, 0));
    assert(!dsu.same(0, 4));
    assert(dsu.same(1, 2));
    assert(dsu.size(dsu.find(0)) == 2 && dsu.size(dsu.find(2)) == 2);
}

int main() {
    RUN_SHORT(unit_test_dsu_rollback());
    return 0;
}
