#include "test_utils.hpp"
#include "struct/mergesort_tree.hpp"

void stress_test_mergesort_tree() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 10'000, runs) {
        print_time(now, 20s, "stress wavelet tree ({} runs)", runs);
        int N = rand_unif<int>(2, 150);
        int MIN = rand_unif<int>(-50, -1);
        int MAX = rand_unif<int>(1, 50);

        vector<int> arr = rands_grav<int>(N, MIN, MAX, 2);
        mergesort_tree<int> mst(arr);

        for (int l = 0; l <= N; l++) {
            for (int r = l; r <= N; r++) {
                for (int x : rands_unif<int>(4, MIN, MAX)) {
                    int less = 0;
                    int less_equal = 0;
                    int greater = 0;
                    int greater_equal = 0;

                    for (int i = l; i < r; i++) {
                        less += arr[i] < x;
                        less_equal += arr[i] <= x;
                        greater += arr[i] > x;
                        greater_equal += arr[i] >= x;
                    }

                    assert(less == mst.count_less(l, r, x));
                    assert(less_equal == mst.count_equal_or_less(l, r, x));
                    assert(greater == mst.count_greater(l, r, x));
                    assert(greater_equal == mst.count_equal_or_greater(l, r, x));
                }
            }
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_mergesort_tree());
    return 0;
}
