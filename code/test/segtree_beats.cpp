#include "test_utils.hpp"
#include "struct/segtree.hpp"
#include "struct/segtree_nodes.hpp"

void stress_test_minupdate_segtree_beats() {
    constexpr int N = 200;
    segtree<setmin_segnode<long>> st(N, 73);
    using Op = setmin_segnode<long>::Operation;
    vector<long> arr(N, 73);

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            int v = rand_unif<int>(1, 1'000'000);
            st.update_beats(L, R, Op::SETMIN, v);
            for (int i = L; i < R; i++) {
                arr[i] = min<long>(arr[i], v);
            }
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            int v = rand_unif<int>(1, 2'000'000);
            st.update_range(L, R, Op::ADD, v);
            for (int i = L; i < R; i++) {
                arr[i] += v;
            }
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            long got = st.query_range(L, R).sum;
            long actual = accumulate(begin(arr) + L, begin(arr) + R, 0LL);
            assert(got == actual);
        }
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            long got = st.query_point(i).sum;
            assert(got == arr[i]);
        }
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            int v = rand_unif<int>(1, 10'000'000);
            st.update_point(i, Op::ADD, v);
            arr[i] += v;
        }
    }
}

void stress_test_modupdate_segtree_beats() {
    constexpr int N = 200, MAX = 500'000'000;
    int X = 100'000'000;
    vector<int> arr = rands_unif<int>(N, 0, MAX);
    segtree<setmod_segnode<long>> st(N, arr);

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            int v = rand_wide<int>(X, MAX, -5);
            st.update_beats(L, R, v);
            for (int i = L; i < R; i++) {
                arr[i] %= v;
            }
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            long got = st.query_range(L, R).sum;
            long actual = accumulate(begin(arr) + L, begin(arr) + R, 0LL);
            assert(got == actual);
        }
        X = max(10'000, X - 50);
    }
}

void stress_test_fullji_segtree_beats() {
    LOOP_FOR_DURATION_TRACKED_RUNS (20s, now, runs) {
        print_time(now, 20s, "stress fullji ({} runs)", runs);
        constexpr int N = 300;

        vector<int64_t> arr = rands_unif<int64_t>(N, -20000, 20000);
        segtree<fullji_segnode<int64_t>> st(N, arr);
        using Operation = fullji_segnode<int64_t>::Operation;

        LOOP_FOR_DURATION (1s) {
            if (cointoss(0.95)) {
                auto [L, R] = ordered_unif<int>(0, N);
                int64_t v = rand_unif<int>(-1000, 1000);
                st.update_beats(L, R, Operation::ADD, v);
                for (int i = L; i < R; i++) {
                    arr[i] += v;
                }
            }
            if (cointoss(0.15)) {
                auto [L, R] = ordered_unif<int>(0, N);
                int64_t v = rand_unif<int>(-1000, 1000);
                st.update_beats(L, R, Operation::SET, v);
                for (int i = L; i < R; i++) {
                    arr[i] = v;
                }
            }
            if (cointoss(0.15)) {
                auto [L, R] = ordered_unif<int>(0, N);
                int64_t v = rand_unif<int>(-1000, 1000);
                st.update_beats(L, R, Operation::SETMAX, v);
                for (int i = L; i < R; i++) {
                    arr[i] = max(arr[i], v);
                }
            }
            if (cointoss(0.15)) {
                auto [L, R] = ordered_unif<int>(0, N);
                int64_t v = rand_unif<int>(-1000, 1000);
                st.update_beats(L, R, Operation::SETMIN, v);
                for (int i = L; i < R; i++) {
                    arr[i] = min(arr[i], v);
                }
            }

            auto [L, R] = different<int>(0, N);
            auto node = st.query_range(L, R);
            int64_t actual_min = *min_element(begin(arr) + L, begin(arr) + R);
            int64_t actual_max = *max_element(begin(arr) + L, begin(arr) + R);
            int64_t actual_sum = accumulate(begin(arr) + L, begin(arr) + R, 0LL);
            assert(node.sum == actual_sum);
            assert(node.maximum == actual_max);
            assert(node.minimum == actual_min);
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_minupdate_segtree_beats());
    RUN_BLOCK(stress_test_modupdate_segtree_beats());
    RUN_BLOCK(stress_test_fullji_segtree_beats());
    return 0;
}
