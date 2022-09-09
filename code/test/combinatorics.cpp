#include "test_utils.hpp"
#include "algo/y_combinator.hpp"
#include "numeric/combinatorics.hpp"
#include "numeric/modnum.hpp"

using num = modnum<998244353>;
using B = Binomial<num>;
using P = Partition<num>;
using D = Permutations<num>;

auto naive_layouts(int n, int k, int a, int b) {
    b = min(b, n);
    return y_combinator([&](auto self, int i, int r) -> num {
        if (i == k) {
            return r == 0;
        }
        int need = (k - i - 1) * a;
        num ans = 0;
        for (int x = a; x <= b && r - x >= need; x++) {
            ans += self(i + 1, r - x);
        }
        return ans;
    })(0, n);
}

void stress_test_layouts() {
    for (int n = 0; n <= 11; n++) {
        for (int k = 0; k <= n + 1; k++) {
            for (int a = 0; a <= n + 1; a++) {
                assert(B::layout(n, k, a) == naive_layouts(n, k, a, n));
                for (int b = a; b <= n + 1; b++) {
                    assert(B::bounded_layout(n, k, a, b) == naive_layouts(n, k, a, b));
                }
            }
        }
    }
}

void unit_test_combinatorics() {
    B::ensure_factorial(16);
    D::ensure_derangements(16);

    println("factorial: {}", B::fact);
    println("invfactorial: {}", B::ifact);
    println("derangements: {}", D::deran);

    P::ensure_partition(32);
    P::ensure_partition_distinct(32);
    P::ensure_partition_odd_distinct(32);
    D::ensure_bell(16);
    D::ensure_stir1st(16);
    D::ensure_stir2nd(16);

    println("partitions: {}", P::partp);
    println("partitions distinct: {}", P::partq);
    println("partitions odd dist: {}", P::partd);
    println("bell: {}", D::belln);
    for (int n = 0; n <= 16; n++) {
        println("stir1st[{:2}]: {}", n, D::stir1st[n]);
    }
    for (int n = 0; n <= 16; n++) {
        println("stir2nd[{:2}]: {}", n, D::stir2nd[n]);
    }
}

int main() {
    RUN_BLOCK(stress_test_layouts());
    RUN_BLOCK(unit_test_combinatorics());
    return 0;
}
