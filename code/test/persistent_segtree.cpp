#include "test_utils.hpp"
#include "struct/persistent_segtree.hpp"
#include "struct/segtree_nodes.hpp"

struct slow_version_array {
    int L, R;
    vector<vector<int>> arrs;

    slow_version_array(int L, int R, const vector<int>& arr) : L(L), R(R), arrs(1) {
        arrs[0].resize(R - L);
        for (int i = L; i < R; i++) {
            arrs[0][i - L] = arr[i];
        }
    }

    int versions() const { return arrs.size(); }

    auto query_range(int v, int qL, int qR) {
        assert(v < versions() && L <= qL && qR <= R);
        return accumulate(begin(arrs[v]) + (qL - L), begin(arrs[v]) + (qR - L), 0);
    }
    auto query_point(int v, int Q) {
        assert(v < versions() && L <= Q && Q < R);
        return arrs[v][Q - L];
    }
    int update_range(int v, int qL, int qR, int add) {
        assert(v < versions() && L <= qL && qR <= R);
        arrs.push_back(arrs[v]);
        for (int i = qL; i < qR; i++) {
            arrs.back()[i - L] += add;
        }
        return arrs.size() - 1;
    }
    int update_point(int v, int Q, int add) {
        assert(v < versions() && L <= Q && Q < R);
        arrs.push_back(arrs[v]);
        arrs.back()[Q - L] += add;
        return arrs.size() - 1;
    }
};

void stress_test_lazy_persistent_segtree() {
    // make two lazy arrays of size SMALL, "arr" and "brr"
    // merge them as arr,brr,brr,arr (size 4*SMALL)
    // arr is array-initialized, brr is sparse initialized
    // then add levels on top, B total, size of 4*SMALL*(1<<B)
    constexpr int B = 3, SMALL = 5;
    constexpr int N = 4 * SMALL * (1 << B);

    vector<int> arr(N, 0);
    for (int i = 0; i < SMALL; i++)
        arr[i] = arr[i + 3 * SMALL] = rand_unif<int>(-20, 20);
    for (int i = 4 * SMALL; i < N; i++)
        arr[i] = arr[i - 4 * SMALL];

    slow_version_array sva(0, N, arr);

    persistent_segtree<sum_segnode> st;
    int root_a = st.build_array(SMALL, arr);
    int root_b = st.build_sparse(0);
    int root_c = st.build_concat({root_a, root_b, root_b, root_a});
    st.add_root(st.build_levels(root_c, B));

    print("initial number of nodes: {}\n", st.num_nodes());

    int start_nodes = st.num_nodes();
    int operations = 0;

    LOOP_FOR_DURATION (1s) {
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            int version = rand_unif<int>(0, st.versions() - 1);
            int v = rand_unif<int>(-10, 10);
            int v1 = st.update_point(version, 0, N, i, v);
            int v2 = sva.update_point(version, i, v);
            assert(v1 == v2);
            operations++;
        }
        if (cointoss(0.5)) {
            int i = rand_unif<int>(0, N - 1);
            int version = rand_unif<int>(0, st.versions() - 1);
            int u1 = st.query_point(version, 0, N, i);
            int u2 = sva.query_point(version, i);
            assert(u1 == u2);
            operations++;
        }
        if (cointoss(0.5)) {
            auto [qL, qR] = different<int>(0, N);
            int version = rand_unif<int>(0, st.versions() - 1);
            int v = rand_unif<int>(-10, 10);
            int v1 = st.update_range(version, 0, N, qL, qR, v);
            int v2 = sva.update_range(version, qL, qR, v);
            assert(v1 == v2);
            operations++;
        }
        if (cointoss(0.5)) {
            auto [qL, qR] = different<int>(0, N);
            int version = rand_unif<int>(0, st.versions() - 1);
            int u1 = st.query_range(version, 0, N, qL, qR);
            int u2 = sva.query_range(version, qL, qR);
            assert(u1 == u2);
            operations++;
        }
        assert(st.versions() == sva.versions());
    }

    double ratio = 1.0 * (st.num_nodes() - start_nodes) / operations;
    print("Final number of ops: {}\n", operations);
    print("Final number of nodes: {}\n", st.num_nodes());
    print("                 N, V: {}, {}\n", N, st.versions());
    print("              V log N: {}\n", st.versions() * log2(N));
    print("            ops log N: {}\n", operations * log2(N));
    print("          nodes / ops: {}\n", ratio);
}

int main() {
    RUN_BLOCK(stress_test_lazy_persistent_segtree());
    return 0;
}
