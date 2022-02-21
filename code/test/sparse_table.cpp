#include "test_utils.hpp"
#include "struct/sparse_table.hpp"
#include "numeric/modnum.hpp"

void stress_test_sparse_table() {
    for (int N = 1; N <= 20; N++) {
        vector<int> A(N);
        for (int i = 0; i < N; i++) {
            A[i] = rand_unif<int>(1, 50) * (1 << rand_unif<int>(0, 5));
        }

        sparse_table st_gcd(A, [&](int u, int v) { return gcd(u, v); });

        for (int a = 0; a < N; a++) {
            for (int b = a + 1; b <= N; b++) {
                int x = st_gcd.query(a, b);
                int X = accumulate(begin(A) + a, begin(A) + b, 0,
                                   [&](int u, int v) { return gcd(u, v); });
                assert(x == X);
            }
        }
    }
}

void stress_test_disjoint_sparse_table() {
    using num = modnum<998244353>;

    for (int N = 1; N <= 20; N++) {
        vector<num> A(N);
        for (int i = 0; i < N; i++) {
            A[i] = 1 << i;
        }

        disjoint_sparse_table sum(A, std::plus<num>{});
        disjoint_sparse_table mul(A, std::multiplies<num>{});

        for (int a = 0; a < N; a++) {
            for (int b = a + 1; b <= N; b++) {
                num x = sum.query(a, b);
                num y = mul.query(a, b);
                num X = accumulate(begin(A) + a, begin(A) + b, num(0));
                num Y = accumulate(begin(A) + a, begin(A) + b, num(1),
                                   std::multiplies<num>{});
                assert(x == X && y == Y);
            }
        }
    }
}

void stress_test_sparse_index_table() {
    for (int N = 1; N <= 20; N++) {
        vector<int> A(N);
        for (int i = 0; i < N; i++) {
            A[i] = rand_unif<int>(1, 50) * (1 << rand_unif<int>(0, 5));
        }

        sparse_index_table minrmq(N, [&](int u, int v) {
            return make_pair(A[u], u) < make_pair(A[v], v) ? u : v;
        });

        for (int a = 0; a < N; a++) {
            for (int b = a + 1; b <= N; b++) {
                int x = minrmq.query(a, b);
                int X = min_element(begin(A) + a, begin(A) + b) - begin(A);
                assert(x == X);
            }
        }
    }
}

void stress_test_sparse_table_2d() {
    for (int N = 1; N <= 20; N++) {
        for (int M = 1; M <= 20; M++) {
            vector<vector<int>> A(N, vector<int>(M));
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    A[i][j] = rand_unif<int>(1, 50000) * (1 << rand_unif<int>(0, 10));
                }
            }

            sparse_table_2d rmq1(A, [&](int u, int v) { return min(u, v); });
            disjoint_sparse_table_2d rmq2(A, [&](int u, int v) { return min(u, v); });

            auto get = [&](int il, int ir, int jl, int jr) {
                int ans = INT_MAX;
                for (int i = il; i < ir; i++) {
                    for (int j = jl; j < jr; j++) {
                        ans = min(ans, A[i][j]);
                    }
                }
                return ans;
            };

            for (int a = 0; a < N; a++) {
                for (int b = a + 1; b <= N; b++) {
                    for (int c = 0; c < M; c++) {
                        for (int d = c + 1; d <= M; d++) {
                            int x1 = rmq1.query(a, b, c, d);
                            int x2 = rmq2.query(a, b, c, d);
                            int X = get(a, b, c, d);
                            assert(x1 == X);
                            assert(x2 == X);
                        }
                    }
                }
            }
        }
    }
}

void stress_test_disjoint_sparse_table_2d() {
    for (int N = 1; N <= 20; N++) {
        for (int M = 1; M <= 20; M++) {
            vector<vector<int>> A(N, vector<int>(M));
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    A[i][j] = rand_unif<int>(1, 500) * (1 << rand_unif<int>(0, 5));
                }
            }

            disjoint_sparse_table_2d rmq(A, std::plus<int>{});

            auto get = [&](int il, int ir, int jl, int jr) {
                int ans = 0;
                for (int i = il; i < ir; i++) {
                    ans += accumulate(begin(A[i]) + jl, begin(A[i]) + jr, 0);
                }
                return ans;
            };

            for (int a = 0; a < N; a++) {
                for (int b = a + 1; b <= N; b++) {
                    for (int c = 0; c < M; c++) {
                        for (int d = c + 1; d <= M; d++) {
                            int x = rmq.query(a, b, c, d);
                            int X = get(a, b, c, d);
                            assert(x == X);
                        }
                    }
                }
            }
        }
    }
}

void speed_test_sparse_table_2d() {
    vector<int> Ns = {60, 100, 150, 220, 300, 400, 500};

    vector<pair<int, int>> inputs;
    for (int N : Ns) {
        for (int M : Ns) {
            if (N * M < 150000) {
                inputs.push_back({N, M});
            }
        }
    }
    map<tuple<int, int, string>, stringable> times;

    for (auto [N, M] : inputs) {
        printcl("speed sparse table N={} M={}", N, M);

        vector<vector<int>> A(N, vector<int>(M));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                A[i][j] = rand_unif<int>(1, 50000) * (1 << rand_unif<int>(0, 10));
            }
        }

        START_ACC2(normal_build, disjoint_build);
        START_ACC2(normal_query, disjoint_query);
        __int128_t ans1 = 0, ans2 = 0;
        int64_t Q = 1LL * N * (N - 1) * M * (M - 1) / 4;

        ADD_TIME_BLOCK(normal_query) {
            START(normal_build);
            sparse_table_2d rmq(A, [](int u, int v) { return min(u, v); });
            ADD_TIME(normal_build);

            for (int a = 0; a < N; a++) {
                for (int b = a + 1; b < N; b++) {
                    for (int c = 0; c < M; c++) {
                        for (int d = c + 1; d < M; d++) {
                            ans1 += rmq.query(a, b, c, d);
                        }
                    }
                }
            }
        }

        ADD_TIME_BLOCK(disjoint_query) {
            START(disjoint_build);
            disjoint_sparse_table_2d rmq(A, [](int u, int v) { return min(u, v); });
            ADD_TIME(disjoint_build);

            for (int a = 0; a < N; a++) {
                for (int b = a + 1; b < N; b++) {
                    for (int c = 0; c < M; c++) {
                        for (int d = c + 1; d < M; d++) {
                            ans2 += rmq.query(a, b, c, d);
                        }
                    }
                }
            }
        }
        assert(ans1 == ans2);

        times[{N, M, "build-normal"}] = FORMAT_TIME(normal_build);
        times[{N, M, "build-disjoint"}] = FORMAT_TIME(disjoint_build);
        times[{N, M, "query-normal"}] = FORMAT_EACH(normal_query, Q);
        times[{N, M, "query-disjoint"}] = FORMAT_EACH(disjoint_query, Q);
    }

    print_time_table(times, "2D Sparse Table");
}

int main() {
    RUN_BLOCK(stress_test_sparse_table());
    RUN_BLOCK(stress_test_disjoint_sparse_table());
    RUN_BLOCK(stress_test_sparse_index_table());
    RUN_BLOCK(stress_test_sparse_table_2d());
    RUN_BLOCK(stress_test_disjoint_sparse_table_2d());
    RUN_BLOCK(speed_test_sparse_table_2d());
    return 0;
}
