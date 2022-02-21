#include "test_utils.hpp"
#include "numeric/kahan.hpp"

void unit_test_kahan() {
    vector<double> nums = {1e5, -1e6, 1e-7, -1e5, 1e6};
    kahan<double> k;
    for (double x : nums) {
        k += x;
    }
    println("k: {}", k.sum);
}

int main() {
    RUN_BLOCK(unit_test_kahan());
    return 0;
}
