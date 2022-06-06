#include "test_utils.hpp"
#include "../struct/pwlc.hpp"

void unit_test_pwlc() {
    using LP = LinPoints<int64_t>;
    LP a = LP::abs(3, 4, 0, 2);
    LP b = LP::abs(5, 2, 3, 1);
    LP c = LP::valley(2, 1, -1, 1, 4);
    putln("a", a.minimum(), a.left_argmin(), a.right_argmin());
    putln("b", b.minimum(), b.left_argmin(), b.right_argmin());
    putln("c", c.minimum(), c.left_argmin(), c.right_argmin());
    LP::pointwise(a, b);
    putln("a+b", a.minimum(), a.left_argmin(), a.right_argmin());
    LP::pointwise(a, c);
    putln("a+b+c", a.minimum(), a.left_argmin(), a.right_argmin());
    putln("a+b+c(2)", a.destructive_query(2));
}

int main() {
    RUN_BLOCK(unit_test_pwlc());
    return 0;
}
