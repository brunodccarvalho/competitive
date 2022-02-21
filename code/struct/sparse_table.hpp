#pragma once

#include "struct/tensor.hpp" // for 2d sparse tables

template <typename T>
struct min_rmq {
    vector<vector<T>> jmp;

    min_rmq() = default;
    explicit min_rmq(const vector<T>& v) : jmp(1, v) {
        for (int len = 1, k = 1, N = v.size(); 2 * len <= N; len *= 2, ++k) {
            int J = N - 2 * len + 1;
            jmp.emplace_back(J);
            for (int j = 0; j < J; j++) {
                const auto& l = jmp[k - 1][j];
                const auto& r = jmp[k - 1][j + len];
                jmp[k][j] = l < r ? l : r; // prefers last
            }
        }
    }

    // query range [a,b)
    T query(int a, int b) const {
        static constexpr int BITS = CHAR_BIT * sizeof(int) - 1;
        // assert(a < b);
        int bits = BITS - __builtin_clz(b - a);
        const auto& l = jmp[bits][a];
        const auto& r = jmp[bits][b - (1 << bits)];
        return l < r ? l : r;
    }
};

template <typename T>
struct min_rmq_index {
    vector<T> v;
    vector<vector<int>> jmp;

    min_rmq_index() = default;
    explicit min_rmq_index(const vector<T>& v) : v(v), jmp(1, vector<int>(v.size())) {
        iota(begin(jmp[0]), end(jmp[0]), 0);
        for (int len = 1, k = 1, N = v.size(); 2 * len <= N; len *= 2, ++k) {
            int J = N - 2 * len + 1;
            jmp.emplace_back(J);
            for (int j = 0; j < J; j++) {
                int l = jmp[k - 1][j];
                int r = jmp[k - 1][j + len];
                jmp[k][j] = v[l] < v[r] ? l : r; // prefers last
            }
        }
    }

    // query range [a,b)
    int query(int a, int b) const {
        static constexpr int BITS = CHAR_BIT * sizeof(int) - 1;
        // assert(a < b);
        int bits = BITS - __builtin_clz(b - a);
        int l = jmp[bits][a];
        int r = jmp[bits][b - (1 << bits)];
        return v[l] < v[r] ? l : r;
    }
};

/**
 * Idempotent queries for ranges [a,b) in O(1) time
 * Memory: O(N log(N))
 */
template <typename T, typename BinOp>
struct sparse_table {
    vector<vector<T>> table;
    BinOp binop;

    template <typename It>
    sparse_table(It first, It last, const BinOp& op) : binop(op) {
        table.emplace_back(first, last);
        int N = table[0].size();
        for (int len = 1, k = 1; 2 * len <= N; len *= 2, k++) {
            int J = N - 2 * len + 1;
            table.emplace_back(J);
            for (int j = 0; j < J; j++) {
                table[k][j] = binop(table[k - 1][j], table[k - 1][j + len]);
            }
        }
    }

    sparse_table(const vector<T>& v, const BinOp& op)
        : sparse_table(begin(v), end(v), op) {}

    // query range [a,b)
    auto query(int a, int b) const {
        static constexpr int BITS = CHAR_BIT * sizeof(int) - 1;
        // assert(0 <= a && a < b && b <= int(table[0].size()));
        if (a + 1 == b) {
            return table[0][a];
        } else {
            int bits = BITS - __builtin_clz(b - a);
            return binop(table[bits][a], table[bits][b - (1 << bits)]);
        }
    }
};

/**
 * Disjoint queries for ranges [a,b) in O(1) time
 * About ~10% slower than normal sparse table
 * Memory: O(N log(N))
 */
template <typename T, typename BinOp>
struct disjoint_sparse_table {
    vector<vector<T>> table;
    BinOp binop;

    template <typename It>
    disjoint_sparse_table(It first, It last, const BinOp& op) : binop(op) {
        table.emplace_back(first, last);
        int N = table[0].size();
        for (int len = 2, h = 1; len <= N; len *= 2, h++) {
            table.emplace_back(N);
            for (int i = len; i <= N; i += 2 * len) { // 000..00 -> 011..11
                table[h][i - 1] = table[0][i - 1];
                for (int j = i - 2; j >= i - len; j--)
                    table[h][j] = binop(table[0][j], table[h][j + 1]);
            }
            for (int i = len; i < N; i += 2 * len) { //  100..00 -> 111..11
                table[h][i] = table[0][i];
                for (int j = i + 1; j < min(N, i + len); j++)
                    table[h][j] = binop(table[h][j - 1], table[0][j]);
            }
        }
    }

    disjoint_sparse_table(const vector<T>& v, const BinOp& op)
        : disjoint_sparse_table(begin(v), end(v), op) {}

    // query range [a,b)
    auto query(int a, int b) const {
        static constexpr int BITS = CHAR_BIT * sizeof(int) - 1;
        // assert(0 <= a && a < b && b <= int(table[0].size()));
        b--;
        if (a == b) {
            return table[0][a];
        } else {
            int bit = BITS - __builtin_clz(a ^ b);
            return binop(table[bit][a], table[bit][b]);
        }
    }
};

/**
 * Idempotent queries for ranges [a,b) in O(1) time, where the op picks one of the indices
 * Memory: O(N log(N))
 */
template <typename BinOp>
struct sparse_index_table {
    vector<vector<int>> table;
    BinOp binop;

    sparse_index_table(int N, const BinOp& op) : binop(op) {
        table.emplace_back(N);
        iota(begin(table[0]), end(table[0]), 0);
        for (int len = 1, k = 1; 2 * len <= N; len *= 2, k++) {
            int J = N - 2 * len + 1;
            table.emplace_back(J);
            for (int j = 0; j < J; j++) {
                table[k][j] = binop(table[k - 1][j], table[k - 1][j + len]);
            }
        }
    }

    // query range [a,b)
    int query(int a, int b) const {
        static constexpr int BITS = CHAR_BIT * sizeof(int) - 1;
        // assert(0 <= a && a < b && b <= int(table[0].size()));
        if (a + 1 == b) {
            return a;
        } else {
            int bits = BITS - __builtin_clz(b - a);
            return binop(table[bits][a], table[bits][b - (1 << bits)]);
        }
    }
};

/**
 * Disjoint queries for ranges [a,b) in O(1) time, where the op picks one of the indices
 * About ~10% slower than normal sparse table
 * Memory: O(N log(N))
 */
template <typename BinOp>
struct disjoint_sparse_index_table {
    vector<vector<int>> table;
    BinOp binop;

    disjoint_sparse_index_table(int N, const BinOp& op) : binop(op) {
        table.emplace_back(N);
        iota(begin(table[0]), end(table[0]), 0);
        for (int len = 2, h = 1; len <= N; len *= 2, h++) {
            table.emplace_back(N);
            for (int i = len; i <= N; i += 2 * len) { // ...000..00 -> ...011..11
                table[h][i - 1] = i - 1;
                for (int j = i - 2; j >= i - len; j--)
                    table[h][j] = binop(j, table[h][j + 1]);
            }
            for (int i = len; i < N; i += 2 * len) { //  ...100..00 -> ...111..11
                table[h][i] = i;
                for (int j = i + 1; j < min(N, i + len); j++)
                    table[h][j] = binop(table[h][j - 1], j);
            }
        }
    }

    // query range [a,b)
    int query(int a, int b) const {
        static constexpr int BITS = CHAR_BIT * sizeof(int) - 1;
        // assert(0 <= a && a < b && b <= int(table[0].size()));
        b--;
        if (a == b) {
            return a;
        } else {
            int bit = BITS - __builtin_clz(a ^ b);
            return binop(table[bit][a], table[bit][b]);
        }
    }
};

/**
 * 2D idempotent queries for rectangles [na,nb)x[ma,mb) in O(1) time
 * Memory: O(NM log(N)log(M))
 */
template <typename T, typename BinOp>
struct sparse_table_2d {
    tensor<T, 4> table;
    BinOp binop;

    sparse_table_2d(const vector<vector<T>>& v, const BinOp& op) : binop(op) {
        int N = v.size(), M = v[0].size();
        table = tensor<T, 4>({log2n(N) + 1, log2n(M) + 1, N, M});

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                table[{0, 0, i, j}] = v[i][j];
            }
        }

        for (int mlen = 1, m = 1; 2 * mlen <= M; mlen *= 2, m++) {
            int MJ = M - 2 * mlen + 1;
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < MJ; j++) {
                    table[{0, m, i, j}] = binop(table[{0, m - 1, i, j}],
                                                table[{0, m - 1, i, j + mlen}]);
                }
            }
        }

        for (int nlen = 1, n = 1; 2 * nlen <= N; nlen *= 2, n++) {
            int NJ = N - 2 * nlen + 1;
            for (int i = 0; i < NJ; i++) {
                for (int j = 0; j < M; j++) {
                    table[{n, 0, i, j}] = binop(table[{n - 1, 0, i, j}],
                                                table[{n - 1, 0, i + nlen, j}]);
                }
            }
            for (int mlen = 1, m = 1; 2 * mlen <= M; mlen *= 2, m++) {
                int MJ = M - 2 * mlen + 1;
                for (int i = 0; i < NJ; i++) {
                    for (int j = 0; j < MJ; j++) {
                        table[{n, m, i, j}] = binop(table[{n - 1, m, i, j}],
                                                    table[{n - 1, m, i + nlen, j}]);
                    }
                }
            }
        }
    }

    static int log2n(int N) { return N > 1 ? 8 * sizeof(int) - __builtin_clz(N - 1) : 0; }

    // query rectangle [na,nb)x[ma,mb)
    auto query(int na, int nb, int ma, int mb) const {
        static constexpr int BITS = CHAR_BIT * sizeof(int) - 1;
        // assert(0 <= na && na < nb && nb <= table.size(2));
        // assert(0 <= ma && ma < mb && mb <= table.size(3));
        int nbits = BITS - __builtin_clz(nb - na);
        int mbits = BITS - __builtin_clz(mb - ma);
        int nx = nb - (1 << nbits);
        int mx = mb - (1 << mbits);
        return binop(binop(table[{nbits, mbits, na, ma}], table[{nbits, mbits, na, mx}]),
                     binop(table[{nbits, mbits, nx, ma}], table[{nbits, mbits, nx, mx}]));
    }
};

/**
 * 2D disjoint queries for rectangles [na,nb)x[ma,mb) in O(1) time
 * About ~10% slower than normal 2d sparse table
 * Memory: O(NM log(N)log(M)) (same as normal 2d sparse table)
 */
template <typename T, typename BinOp>
struct disjoint_sparse_table_2d {
    tensor<T, 4> table;
    BinOp binop;

    disjoint_sparse_table_2d(const vector<vector<T>>& v, const BinOp& op) : binop(op) {
        int N = v.size(), M = v[0].size();
        table = tensor<T, 4>({log2n(N) + 1, log2n(M) + 1, N, M});

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                table[{0, 0, i, j}] = v[i][j];
            }
        }

        for (int mlen = 2, m = 1; mlen <= M; mlen *= 2, m++) {
            for (int x = 0; x < N; x++) {
                for (int i = mlen; i <= M; i += 2 * mlen) { // 000..00 -> 011..11
                    table[{0, m, x, i - 1}] = table[{0, 0, x, i - 1}];
                    for (int j = i - 2; j >= i - mlen; j--)
                        table[{0, m, x, j}] = binop(table[{0, 0, x, j}],
                                                    table[{0, m, x, j + 1}]);
                }
                for (int i = mlen; i < M; i += 2 * mlen) { //  100..00 -> 111..11
                    table[{0, m, x, i}] = table[{0, 0, x, i}];
                    for (int j = i + 1; j < min(M, i + mlen); j++)
                        table[{0, m, x, j}] = binop(table[{0, m, x, j - 1}],
                                                    table[{0, 0, x, j}]);
                }
            }
        }

        for (int nlen = 2, n = 1; nlen <= N; nlen *= 2, n++) {
            for (int y = 0; y < M; y++) {
                for (int i = nlen; i <= N; i += 2 * nlen) { // 000..00 -> 011..11
                    table[{n, 0, i - 1, y}] = table[{0, 0, i - 1, y}];
                    for (int j = i - 2; j >= i - nlen; j--)
                        table[{n, 0, j, y}] = binop(table[{0, 0, j, y}],
                                                    table[{n, 0, j + 1, y}]);
                }
                for (int i = nlen; i < N; i += 2 * nlen) { //  100..00 -> 111..11
                    table[{n, 0, i, y}] = table[{0, 0, i, y}];
                    for (int j = i + 1; j < min(N, i + nlen); j++)
                        table[{n, 0, j, y}] = binop(table[{n, 0, j - 1, y}],
                                                    table[{0, 0, j, y}]);
                }
            }
            for (int mlen = 2, m = 1; mlen <= M; mlen *= 2, m++) {
                for (int y = 0; y < M; y++) {
                    for (int i = nlen; i <= N; i += 2 * nlen) { // 000..00 -> 011..11
                        table[{n, m, i - 1, y}] = table[{0, m, i - 1, y}];
                        for (int j = i - 2; j >= i - nlen; j--)
                            table[{n, m, j, y}] = binop(table[{0, m, j, y}],
                                                        table[{n, m, j + 1, y}]);
                    }
                    for (int i = nlen; i < N; i += 2 * nlen) { //  100..00 -> 111..11
                        table[{n, m, i, y}] = table[{0, m, i, y}];
                        for (int j = i + 1; j < min(N, i + nlen); j++)
                            table[{n, m, j, y}] = binop(table[{n, m, j - 1, y}],
                                                        table[{0, m, j, y}]);
                    }
                }
            }
        }
    }

    static int log2n(int N) { return N > 1 ? 8 * sizeof(int) - __builtin_clz(N - 1) : 0; }

    // query rectangle [na,nb)x[ma,mb)
    auto query(int na, int nb, int ma, int mb) const {
        static constexpr int BITS = CHAR_BIT * sizeof(int) - 1;
        // assert(0 <= na && na < nb && nb <= table.size(2));
        // assert(0 <= ma && ma < mb && mb <= table.size(3));
        nb--, mb--;
        if (na == nb && ma == mb) {
            return table[{0, 0, na, ma}];
        } else if (na == nb) {
            int mbit = BITS - __builtin_clz(ma ^ mb);
            return binop(table[{0, mbit, na, ma}], table[{0, mbit, nb, mb}]);
        } else if (ma == mb) {
            int nbit = BITS - __builtin_clz(na ^ nb);
            return binop(table[{nbit, 0, na, ma}], table[{nbit, 0, nb, mb}]);
        } else {
            int nbit = BITS - __builtin_clz(na ^ nb);
            int mbit = BITS - __builtin_clz(ma ^ mb);
            return binop(binop(table[{nbit, mbit, na, ma}], table[{nbit, mbit, na, mb}]),
                         binop(table[{nbit, mbit, nb, ma}], table[{nbit, mbit, nb, mb}]));
        }
    }
};
