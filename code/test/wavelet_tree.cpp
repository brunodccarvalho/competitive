#include "test_utils.hpp"
#include "struct/wavelet_tree.hpp"

void stress_test_wavelet_tree() {
    mt.seed(73);
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 10'000, runs) {
        print_time(now, 20s, "stress wavelet tree ({} runs)", runs);
        int N = rand_unif<int>(2, 30);
        int MIN = 0;
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

                for (int x = MIN + 1; x < MAX; x++) {
                    cnt[x] += cnt[x - 1];
                }

                for (int k = 0; k < r - l; k++) {
                    assert(wvl.find_by_order(l, r, k) == brr[k]);
                }

                for (int x = MIN; x <= MAX; x++) {
                    for (int y = x; y <= MAX; y++) {
                        int c = x < y ? cnt[y - 1] - cnt[x - 1] : 0;
                        int d = wvl.count_within(l, r, x, y);
                        if (c != d) {
                            debug(l, r, MIN, MAX, x, y, c, d);
                        }
                        assert(c == d);
                    }
                }
            }
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_wavelet_tree());
    return 0;
}
