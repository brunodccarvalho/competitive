#include "test_utils.hpp"
#include "struct/wavelet_tree.hpp"

void stress_test_wavelet_tree() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 10'000, runs) {
        print_time(now, 20s, "stress wavelet tree ({} runs)", runs);
        int N = rand_unif<int>(2, 150);
        int MIN = rand_unif<int>(-50, -1);
        int MAX = rand_unif<int>(1, 50);

        vector<int> arr = rands_grav<int>(N, MIN, MAX - 1, 2);
        wavelet_tree wvl(MIN, MAX, arr);

        for (int l = 0; l <= N; l++) {
            for (int r = l; r <= N; r++) {
                map<int, int> cnt;
                for (int i = l; i < r; i++) {
                    cnt[arr[i]]++;
                }

                vector<int> brr(begin(arr) + l, begin(arr) + r);
                sort(begin(brr), end(brr));

                for (int x = MIN; x < MAX; x++) {
                    assert(wvl.count_equal(l, r, x) == cnt[x]);
                }

                for (int x = MIN + 1; x < MAX; x++) {
                    cnt[x] += cnt[x - 1];
                }

                for (int x = MIN; x <= MAX; x++) {
                    assert(wvl.order_of_key(l, r, x) == cnt[x - 1]);
                }

                for (int k = 0; k < r - l; k++) {
                    assert(wvl.find_by_order(l, r, k) == brr[k]);
                }
            }
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_wavelet_tree());
    return 0;
}
