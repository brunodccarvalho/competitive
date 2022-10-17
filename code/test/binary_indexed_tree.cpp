#include "test_utils.hpp"
#include "struct/binary_indexed_tree.hpp"

void stress_test_bitree() {
    constexpr int N = 50, RUNS = 10'000'000, MAX = (1 << 25) - 1;

    vector<int> arr(N);
    for (int i = 0; i < N; i++) {
        arr[i] = (i + 1) ^ 1337;
    }

    bitree<int, bit_xor<int>> bit(N, arr, bit_xor<int>{}, 0);

    for (int run = 0; run < RUNS; run++) {
        print_regular(run, RUNS, 10'000, "stress test bitree");

        int r = rand_unif<int>(0, N - 1);
        int v = rand_unif<int>(0, MAX);
        bit.combine(r, v);
        arr[r] ^= v;

        r = rand_unif<int>(1, N);
        int x = bit.prefix(r);
        int y = 0;
        for (int i = 0; i < r; i++) {
            y ^= arr[i];
        }
        assert(x == y);
    }
}

void stress_test_bitree2d() {
    constexpr int N = 20, M = 20, RUNS = 10'000'000, MAX = (1 << 25) - 1;

    vector<vector<int>> arr(N, vector<int>(M));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (i + 1) ^ (2 * j + 1);
        }
    }

    bitree2d<int, bit_xor<int>> bit(N, M, arr, bit_xor<int>{}, 0);

    for (int run = 0; run < RUNS; run++) {
        print_regular(run, RUNS, 10'000, "stress test bitree2d");

        int r = rand_unif<int>(0, N - 1);
        int c = rand_unif<int>(0, M - 1);
        int v = rand_unif<int>(0, MAX);
        bit.combine(r, c, v);
        arr[r][c] ^= v;

        r = rand_unif<int>(1, N);
        c = rand_unif<int>(1, M);
        int x = bit.prefix(r, c);
        int y = 0;
        for (int i = 0; i < r; i++) {
            for (int j = 0; j < c; j++) {
                y ^= arr[i][j];
            }
        }
        assert(x == y);
    }
}

int main() {
    RUN_BLOCK(stress_test_bitree());
    RUN_BLOCK(stress_test_bitree2d());
    return 0;
}
