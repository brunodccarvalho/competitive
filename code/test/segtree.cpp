#include "test_utils.hpp"
#include "struct/segtree.hpp"
#include "struct/segtree_nodes.hpp"

void stress_test_gcd_segtree() {
    constexpr int N = 200;

    vector<int> arr(N);
    segtree<gcd_segnode> st(N, 0);

    LOOP_FOR_DURATION (1s) {
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            int v = rand_unif<int>(-50, 50);
            st.update_range(L, R, v);
            for (int i = L; i < R; i++) {
                arr[i] += v;
            }
        }
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            int got = st.query_range(L, R);
            int actual = 0;
            for (int i = L; i < R; i++) {
                actual = gcd(actual, arr[i]);
            }
            assert(got == actual);
        }
    }
}

void stress_test_maxcount_segtree() {
    constexpr int N = 200;

    vector<int> arr(N, 0);
    segtree<maxcount_segnode> st(N, 0);

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            int v = rand_unif<int>(-50, 50);
            st.update_range(L, R, v);
            for (int i = L; i < R; i++) {
                arr[i] += v;
            }
        }
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            auto ans = st.query_range(L, R);
            auto got_max = ans.value;
            auto got_cnt = ans.count;

            int actual_max = INT_MIN, actual_cnt = 0;
            for (int i = L; i < R; i++) {
                if (arr[i] > actual_max) {
                    actual_max = arr[i];
                    actual_cnt = 1;
                } else if (arr[i] == actual_max) {
                    actual_cnt++;
                }
            }
            assert(got_max == actual_max && got_cnt == actual_cnt);
        }
    }
}

void stress_test_maxsubrange_segtree() {
    constexpr int N = 200;

    vector<int> arr(N, 0);
    segtree<maxsubrange_segnode> st(N, 0);

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            int v = rand_unif<int>(-50, 50);
            st.update_point(i, v);
            arr[i] += v;
        }
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            int got = st.query_range(L, R);

            int actual = INT_MIN;
            for (int i = L; i < R; i++) {
                int sum = arr[i];
                actual = max(actual, sum);
                for (int j = i + 1; j < R; j++) {
                    sum += arr[j];
                    actual = max(actual, sum);
                }
            }
            assert(got == actual);
        }
    }
}

void stress_test_maxsubrange_iterative_segtree() {
    constexpr int N = 200;

    vector<int> arr(N, 0);
    segtree<maxsubrange_segnode> st(N, 0);

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            int v = rand_unif<int>(-50, 50);
            st.update_point(i, v);
            arr[i] += v;
        }
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            int got = st.query_range(L, R);

            int actual = INT_MIN;
            for (int i = L; i < R; i++) {
                int sum = arr[i];
                actual = max(actual, sum);
                for (int j = i + 1; j < R; j++) {
                    sum += arr[j];
                    actual = max(actual, sum);
                }
            }
            assert(got == actual);
        }
    }
}

void stress_test_affine_segtree() {
    constexpr int N = 200;

    using num = modnum<998244353>;
    using Data = affine_segnode::Data;
    vector<Data> arr(N, {1, 0});
    segtree<affine_segnode> st(N, Data{1, 0});

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            num b = rand_unif<int, num>(0, 1'000'000);
            num c = rand_unif<int, num>(-1000, 1000);
            st.update_point(i, Data{b, c});
            arr[i] = {b, c};
        }
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            auto got = st.query_range(L, R);

            num x = rand_unif<int, num>(0, 1'000'000);
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

void stress_test_polyhash_segtree() {
    constexpr int N = 200;

    using num = modnum<998244353>;
    polyhash_segnode::init(N, 2);
    segtree<polyhash_segnode> st(N, num(0));
    vector<num> arr(N, 0);

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            num v = rand_unif<int, num>(0, 70000);
            st.update_point(i, v);
            arr[i] = v;
        }
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            num got = st.query_range(L, R);

            num actual = 0;
            for (int i = L; i < R; i++) {
                actual += arr[i] * polyhash_segnode::polypow[i - L];
            }
            assert(got == actual);
        }
    }
}

void speed_test_segtree() {
    constexpr int N = 500'000;

    segtree<Segnode> st(N, 0);

    LOOP_FOR_DURATION_TRACKED_RUNS (3s, now, runs) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            int v = rand_unif<int>(-100, 100);
            st.update_point(i, v);
        }
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            int v = rand_unif<int>(-10, 10);
            st.update_range(L, R, v);
        }
        if (cointoss(0.5)) {
            auto [L, R] = different<int>(0, N);
            [[maybe_unused]] auto got = st.query_range(L, R);
        }
    }

    printcl("runs: {}\n", runs);
}

int main() {
    RUN_BLOCK(stress_test_gcd_segtree());
    RUN_BLOCK(stress_test_maxcount_segtree());
    RUN_BLOCK(stress_test_maxsubrange_segtree());
    RUN_BLOCK(stress_test_maxsubrange_iterative_segtree());
    RUN_BLOCK(stress_test_affine_segtree());
    RUN_BLOCK(stress_test_polyhash_segtree());
    RUN_BLOCK(speed_test_segtree());
    return 0;
}
