#include "test_utils.hpp"
#include "numeric/combinatorics.hpp"
#include "numeric/modnum.hpp"

using num = modnum<998244353>;
using B = Binomial<num>;
using P = Partition<num>;
using D = Permutations<num>;

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

    println("partitions: {}", P::partp);
    println("partitions distinct: {}", P::partq);
    println("partitions odd dist: {}", P::partd);
    println("bell: {}", D::belln);
}

int main() {
    RUN_BLOCK(unit_test_combinatorics());
    return 0;
}
