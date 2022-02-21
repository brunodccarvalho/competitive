#include "test_utils.hpp"
#include "struct/dynamic_cht.hpp"

void unit_test_dynamic_cht() {
    dynamic_cht<long> cht;
    cht.add(2, -3);
    cht.add(4, -70);
    cht.add(-20, -500);
    cht.add(-5, -100);
    cht.add(-8, -200);
    cht.add(10, -100);
    cht.add(-1, 100);
    cout << cht.size() << endl;
    for (auto [m, b, end] : cht) {
        print("({}, {}, {})\n", m, b, end);
    }
}

int main() {
    RUN_SHORT(unit_test_dynamic_cht());
    return 0;
}
