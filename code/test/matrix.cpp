#include "test_utils.hpp"
#include "linear/matrix.hpp"
#include "linear/gauss.hpp"
#include "numeric/modnum.hpp"

void unit_test_matrix() {
    using num = modnum<7>;
    mat<num> a = mat<num>::from<int>({
        {0, 3, 5, 1},
        {2, 1, 4, 6},
        {3, 0, 0, 2},
    });
    vector<num> b = {1, 2, 3};

    if (auto x = solve_linear_system(a, b); !x.empty()) {
        println("x: {}", x);
        assert(a * x == b);
    } else {
        println("ans: none");
    }
}

int main() {
    RUN_BLOCK(unit_test_matrix());
    return 0;
}
