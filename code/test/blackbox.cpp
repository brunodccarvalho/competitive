#include "test_utils.hpp"
#include "linear/blackbox/matrix.hpp"

using num = modnum<998244353>;

auto wolfram_circulant_matrix(vector<int> v) {
    string s;
    for (int i = 0, n = v.size(); i < n; i++) {
        s += "(" + format("{}", fmt::join(v, ",")) + "),";
        rotate(begin(v), end(v) - 1, end(v));
    }
    s.pop_back();
    return '(' + s + ')';
}

void unit_test_blackbox() {
    outer_product_matrix<int> M1({1, 2, 3}, {4, 5, 6});
    vector<int> x1 = {7, 8, 9};
    putln(M1 * x1);

    circulant_matrix<num> M2(8, {1, 2, 3, 4, 5, 6, 7, 8});
    vector<num> x2 = {1, -1, 2, -2, 3, -3, 4, -4};
    putln(M2 * x2);
    putln(wolfram_circulant_matrix({1, 2, 3, 4, 5, 6, 7, 8}));
    putln(matrix_minimal_polynomial<num>(M2));
    putln(matrix_determinant<num>(M2));

    circulant_matrix<num> M3(7, {1, 2, 3, 4, 5, 6, 7});
    vector<num> x3 = {1, -1, 2, -2, 3, -3, 4};
    putln(M3 * x3);
    putln(wolfram_circulant_matrix({1, 2, 3, 4, 5, 6, 7}));
    putln(matrix_minimal_polynomial<num>(M3));
    putln(matrix_determinant<num>(M3));
}

int main() {
    RUN_BLOCK(unit_test_blackbox());
    return 0;
}
