#include "test_utils.hpp"
#include "struct/dynamic_segtree.hpp"
#include "struct/segtree_nodes.hpp"

void stress_test_dynamic_segtree() {
    // make two lazy arrays of size SMALL, arr[] and brr[]
    // merge them as arr,brr,brr,arr (size 4*SMALL)
    // arr is array-initialized, brr is sparse initialized
    // then add levels on top, B total, size of 4*SMALL*(1<<B)
    constexpr int B = 5, SMALL = 40;
    constexpr int N = 4 * SMALL * (1 << B);
    constexpr int L = 0, R = N;

    vector<int> arr(R, 0);
    for (int i = 0; i < SMALL; i++)
        arr[i] = arr[i + 3 * SMALL] = rand_unif<int>(-10, 10);
    for (int i = 4 * SMALL; i < R; i++)
        arr[i] = arr[i - 4 * SMALL];

    dynamic_segtree<sum_segnode> st;
    int root_a = st.build_array(SMALL, arr, true);
    int root_b = st.build_sparse(0, true);
    int root_c = st.build_concat({root_a, root_b, root_b, root_a}, true);
    int root = st.build_levels(root_c, B, false);

    print("initial number of nodes: {}\n", st.num_nodes());

    LOOP_FOR_DURATION (2s) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            int v = rand_unif<int>(-10, 10);
            st.update_point(root, L, R, i, v);
            arr[i] += v;
        }
        if (cointoss(0.5)) {
            auto [qL, qR] = ordered_unif<int>(0, N);
            int v = rand_unif<int>(-10, 10);
            st.update_range(root, L, R, qL, qR, v);
            for (int i = qL; i < qR; i++) {
                arr[i] += v;
            }
        }
        if (cointoss(0.5)) {
            auto [qL, qR] = ordered_unif<int>(0, N);
            long got = st.query_range(root, L, R, qL, qR).value;
            long actual = accumulate(begin(arr) + qL, begin(arr) + qR, 0LL);
            assert(got == actual);
        }
    }

    print("final number of nodes: {}\n", st.num_nodes());
    print("       normal segtree: {}\n", 2 * (R - L));
}

void stress_test_maxsubrange_segtree() {
    constexpr int N = 200;
    using V = maxsubrange_segnode::V;

    vector<V> arr(N, 0);
    dynamic_segtree<maxsubrange_segnode> st;
    int root = st.build_sparse({0}, false);

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            int v = rand_unif<int>(-50, 50);
            st.update_point(root, 0, N, i, v);
            arr[i] += v;
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            auto got = st.query_range(root, 0, N, L, R);

            auto actual = maxsubrange_segnode::MIN;
            for (int i = L; i < R; i++) {
                auto sum = arr[i];
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
    dynamic_segtree<affine_segnode> st;
    int root = st.build_sparse(Data{1, 0}, false);

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            num b = rand_unif<int, num>(0, 500);
            num c = rand_unif<int, num>(-1000, 1000);
            st.update_point(root, 0, N, i, Data{b, c});
            arr[i] = {b, c};
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            auto got = st.query_range(root, 0, N, L, R);

            num x = rand_unif<int, num>(1, 73);
            auto got_lmr = got.eval_lmr(x);
            auto got_rml = got.eval_rml(x);

            num actual_lmr = x;
            num actual_rml = x;
            for (int i = L; i < R; i++) {
                actual_rml = arr[i][0] * actual_rml + arr[i][1];
            }
            for (int i = R - 1; i >= L; i--) {
                actual_lmr = arr[i][0] * actual_lmr + arr[i][1];
            }
            assert(got_lmr == actual_lmr);
            assert(got_rml == actual_rml);
        }
    }
}

void stress_test_gcd_segtree() {
    constexpr int N = 200;

    vector<int> arr(N);
    dynamic_segtree<gcd_segnode> st;
    int root = st.build_sparse({0}, false);

    LOOP_FOR_DURATION (1s) {
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            int v = rand_unif<int>(-10, 10);
            st.update_range(root, 0, N, L, R, v);
            for (int i = L; i < R; i++) {
                arr[i] += v;
            }
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            int got = st.query_range(root, 0, N, L, R);
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
    using V = maxcount_segnode::V;

    vector<V> arr(N, 0);
    dynamic_segtree<maxcount_segnode> st;
    int root = st.build_array(N, arr, false);

    LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            int v = rand_unif<int>(-50, 50);
            st.update_range(root, 0, N, L, R, v);
            for (int i = L; i < R; i++) {
                arr[i] += v;
            }
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            auto ans = st.query_range(root, 0, N, L, R);
            auto got_max = ans.value;
            auto got_cnt = ans.count;

            auto actual_max = maxcount_segnode::MIN;
            int actual_cnt = 0;
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

void speed_test_affine_dynamic_segtree() {
    constexpr int N = 500'000;

    using num = modnum<998244353>;
    using Data = affine_segnode::Data;
    dynamic_segtree<affine_segnode> st;
    int root = st.build_sparse(Data{1, 0}, false);

    LOOP_FOR_DURATION_TRACKED_RUNS (3s, now, runs) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            num b = rand_unif<int, num>(0, 500);
            num c = rand_unif<int, num>(-1000, 1000);
            st.update_point(root, 0, N, i, Data{b, c});
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            auto got = st.query_range(root, 0, N, L, R);

            num x = rand_unif<int, num>(1, 73);
            got.eval_lmr(x);
            got.eval_rml(x);
        }
    }

    println("runs: {}\n", runs);
}

void speed_test_dynamic_segtree() {
    constexpr int N = 500'000;

    dynamic_segtree<sum_segnode> st;
    int root = st.build_sparse({0}, false);

    LOOP_FOR_DURATION_TRACKED_RUNS (3s, now, runs) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            int v = rand_unif<int>(0, 100);
            st.update_point(root, 0, N, i, v);
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            int v = rand_unif<int>(0, 10);
            st.update_range(root, 0, N, L, R, v);
        }
        if (cointoss(0.5)) {
            auto [L, R] = ordered_unif<int>(0, N);
            st.query_range(root, 0, N, L, R);
        }
    }

    printcl("runs: {}\n", runs);
}

int main() {
    RUN_BLOCK(stress_test_dynamic_segtree());
    RUN_BLOCK(stress_test_maxsubrange_segtree());
    RUN_BLOCK(stress_test_affine_segtree());
    RUN_BLOCK(stress_test_gcd_segtree());
    RUN_BLOCK(stress_test_maxcount_segtree());
    RUN_BLOCK(speed_test_affine_dynamic_segtree());
    RUN_BLOCK(speed_test_dynamic_segtree());
    return 0;
}
