#include "test_utils.hpp"
#include "../struct/slopy.hpp"

void unit_test_slopy() {
    auto test_slopes = [&](const vector<array<int, 2>>& slopes) {
        Slopy fn;

        for (auto [s, len] : slopes) {
            Branch::insert_segment(fn.tree, s, len);
        }

        vector<int> value = {0};
        for (auto [s, len] : slopes) {
            while (len--) {
                value.push_back(value.back() + s);
            }
        }
        int V = value.size();
        int A = *min_element(begin(value), end(value));
        int B = *max_element(begin(value), end(value));

        for (int ceiling = A - 3; ceiling <= B + 3; ceiling++) {
            auto [l, r] = Branch::ceiling_range(fn.tree, ceiling);
            int L = V, R = 0;
            for (int i = 0; i < V; i++) {
                if (value[i] <= ceiling) {
                    L = min(i, L);
                    R = max(i, R);
                }
            }
            if (ceiling < A) {
                assert(l == -1 && r == -1);
            } else {
                assert(L == l && R == r);
            }
        }

        assert(V >= 12);
        int R = V - 1;

        for (int i = 0; i < V; i++) {
            assert(fn.query(i) == value[i]);
        }
        fn.trimto(5, R - 5);
        for (int i = 5; i <= R - 5; i++) {
            assert(fn.query(i) == value[i]);
        }
    };

    test_slopes({
        {-6, 10}, //
        {-5, 3},  //
        {-4, 11}, //
        {-2, 13}, //
        {-1, 4},  //
        {0, 9},   //
        {2, 5},   //
        {5, 6},   //
        {7, 6},   //
        {12, 11}, //
    });

    test_slopes({
        {-6, 10}, //
        {-5, 3},  //
        {2, 5},   //
        {7, 6},   //
        {12, 11}, //
    });

    test_slopes({
        {0, 9},   //
        {2, 5},   //
        {5, 6},   //
        {7, 6},   //
        {12, 11}, //
    });

    test_slopes({
        {-6, 10}, //
        {-5, 3},  //
        {-4, 11}, //
        {-2, 13}, //
        {-1, 4},  //
        {0, 9},   //
    });

    test_slopes({
        {2, 5},   //
        {5, 6},   //
        {7, 6},   //
        {12, 11}, //
    });

    test_slopes({
        {-6, 10}, //
        {-5, 3},  //
        {-4, 11}, //
        {-2, 13}, //
        {-1, 4},  //
    });
}

int main() {
    RUN_BLOCK(unit_test_slopy());
    return 0; //
}
