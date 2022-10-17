#include "test_utils.hpp"
#include "struct/splay_segtree.hpp"
#include "struct/splay_segnodes.hpp"
#include "numeric/modnum.hpp"

void stress_test_sum_splay_segtree() {
    constexpr int N = 200;

    LOOP_FOR_DURATION (5s) {
        vector<int> arr(N);
        splay_segtree<sum_segnode<int>> st(0, N);

        LOOP_FOR_DURATION (25ms) {
            if (cointoss(0.5)) {
                int i = rand_unif<int>(0, N - 1);
                int v = rand_unif<int>(-5, 5);
                st.update_point(i, v);
                arr[i] += v;
            }
            if (cointoss(0.5)) {
                int i = rand_unif<int>(0, N - 1);
                int got = st.query_point(i).point_val();
                int actual = arr[i];
                assert(got == actual);
            }
            if (cointoss(0.5)) {
                auto [L, R] = diff_unif<int>(0, N);
                int v = rand_unif<int>(-5, 5);
                st.update_range(L, R, v);
                for (int i = L; i < R; i++) {
                    arr[i] += v;
                }
            }
            if (cointoss(0.5)) {
                auto [L, R] = diff_unif<int>(0, N);
                int got = st.query_range(L, R).range_sum();
                int actual = accumulate(begin(arr) + L, begin(arr) + R, 0);
                assert(got == actual);
            }
            if (cointoss(0.5)) {
                int v = rand_unif<int>(-5, 5);
                st.update_all(v);
                for (int i = 0; i < N; i++) {
                    arr[i] += v;
                }
            }
            if (cointoss(0.5)) {
                int got = st.query_all().range_sum();
                int actual = accumulate(begin(arr), end(arr), 0);
                assert(got == actual);
            }
        }
    }
}

void stress_test_sum_affine_splay_segtree() {
    constexpr int N = 200;
    using num = modnum<998244353>;

    LOOP_FOR_DURATION (5s) {
        vector<num> arr(N);
        splay_segtree<sum_affine_segnode<num>> st(0, N);

        LOOP_FOR_DURATION (25ms) {
            if (cointoss(0.5)) {
                auto [L, R] = diff_unif<int>(0, N);
                num b = rand_unif<int>(-1000, 1000);
                num c = rand_unif<int>(-1000, 1000);

                st.update_range(L, R, make_pair(b, c));
                for (int i = L; i < R; i++) {
                    arr[i] = b * arr[i] + c;
                }
            }
            if (cointoss(0.5)) {
                auto [L, R] = diff_unif<int>(0, N);
                num got = st.query_range(L, R).range_sum();
                num actual = accumulate(begin(arr) + L, begin(arr) + R, num(0));
                assert(got == actual);
            }
            if (cointoss(0.5)) {
                num b = rand_unif<int>(-1000, 1000);
                num c = rand_unif<int>(-1000, 1000);
                st.update_all(make_pair(b, c));
                for (int i = 0; i < N; i++) {
                    arr[i] = b * arr[i] + c;
                }
            }
            if (cointoss(0.5)) {
                num got = st.query_all().range_sum();
                num actual = accumulate(begin(arr), end(arr), num(0));
                assert(got == actual);
            }
        }
    }
}

void stress_test_min_splay_segtree() {
    constexpr int N = 200;

    LOOP_FOR_DURATION (5s) {
        vector<int> arr(N, 0);
        splay_segtree<min_segnode<int>> st(0, N, min_segnode<int>(0));

        LOOP_FOR_DURATION (100ms) {
            if (cointoss(0.5)) {
                auto [L, R] = diff_unif<int>(0, N);
                int v = rand_unif<int>(-50, 50);
                st.update_range(L, R, v);
                for (int i = L; i < R; i++) {
                    arr[i] += v;
                }
            }
            if (cointoss(0.5)) {
                auto [L, R] = diff_unif<int>(0, N);
                int got = st.query_range(L, R).range_min();
                int actual = *min_element(begin(arr) + L, begin(arr) + R);
                assert(got == actual);
            }
        }
    }
}

void stress_test_affine_splay_segtree() {
    constexpr int N = 200;
    using num = modnum<998244353>;
    using Data = affine_segnode<num>::Data;

    LOOP_FOR_DURATION (5s) {
        vector<Data> arr(N, {1, 0});
        splay_segtree<affine_segnode<num>> st(0, N);

        LOOP_FOR_DURATION_TRACKED_RUNS (100ms, now, runs) {
            if (cointoss(0.5)) {
                int i = rand_unif<int>(0, N - 1);
                num b = rand_unif<int>(-1000, 1000);
                num c = rand_unif<int>(-1000, 1000);
                st.update_point(i, Data{b, c});
                arr[i] = {b, c};
            }
            if (cointoss(0.5)) {
                auto [L, R] = diff_unif<int>(0, N);
                auto got = st.query_range(L, R);

                num x = rand_unif<int>(-100, 100);
                auto got_lmr = got.eval_lmr(x);
                auto got_rml = got.eval_rml(x);

                num expected_lmr = x;
                num expected_rml = x;
                for (int i = L; i < R; i++) {
                    expected_rml = arr[i][0] * expected_rml + arr[i][1];
                }
                for (int i = R - 1; i >= L; i--) {
                    expected_lmr = arr[i][0] * expected_lmr + arr[i][1];
                }
                assert(got_lmr == expected_lmr);
                assert(got_rml == expected_rml);
            }
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_sum_splay_segtree());
    RUN_BLOCK(stress_test_sum_affine_splay_segtree());
    RUN_BLOCK(stress_test_min_splay_segtree());
    RUN_BLOCK(stress_test_affine_splay_segtree());
    return 0;
}
