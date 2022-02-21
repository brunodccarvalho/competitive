#pragma once

#include <bits/stdc++.h>
using namespace std;

// tmaehara's submission on yosupo
template <typename Cost = int64_t, typename CostSum = Cost>
auto fast_dense_hungarian(const vector<vector<Cost>>& cost) {
    static constexpr CostSum inf = numeric_limits<CostSum>::max();
    int rows = cost.size();
    int cols = cost[0].size();

    vector<int> row_mate(rows, -1);
    vector<int> col_mate(cols, -1);
    vector<CostSum> pi(cols, 0);

    auto residual = [&](int r, int c) { return cost[r][c] - pi[c]; };

    // column reduction, mate columns greedily
    vector<bool> transferrable(rows, false);
    for (int col = 0; col < cols; col++) {
        int row = 0;
        for (int u = 1; u < rows; u++) {
            if (cost[row][col] > cost[u][col]) {
                row = u;
            }
        }
        pi[col] = cost[row][col];
        if (row_mate[row] == -1) {
            row_mate[row] = col;
            col_mate[col] = row;
            transferrable[row] = true;
        } else {
            transferrable[row] = false;
        }
    }

    // reduction transfer
    for (int row = 0; row < rows; row++) {
        if (transferrable[row]) {
            int col = row_mate[row];
            int c = -1;
            for (int v = 0; v < cols; v++) {
                if (v != col && (c == -1 || residual(row, c) > residual(row, v))) {
                    c = v;
                }
            }
            pi[col] -= residual(row, c);
        }
    }

    // augmenting row reduction
    for (int i = 0; i <= 1; i++) {
        for (int row = 0; row < rows; row++) {
            if (row_mate[row] != -1) {
                continue;
            }
            auto u1 = residual(row, 0);
            auto u2 = inf;
            int c1 = 0;
            for (int c = 0; c < cols; c++) {
                auto u = residual(row, c);
                if (u < u1 || (u == u1 && col_mate[c1] != -1)) {
                    u2 = u1, u1 = u, c1 = c;
                } else if (u < u2) {
                    u2 = u;
                }
            }
            if (u1 < u2) {
                pi[c1] -= u2 - u1;
            }
            if (int r1 = col_mate[c1]; r1 != -1) {
                row_mate[r1] = col_mate[c1] = -1;
            }
            row_mate[row] = c1;
            col_mate[c1] = row;
        }
    }

    vector<int> columns(cols);
    iota(begin(columns), end(columns), 0);

    for (int row = 0; row < rows; row++) {
        if (row_mate[row] != -1) {
            continue;
        }
        vector<CostSum> dist(cols);
        for (int c = 0; c < cols; c++) {
            dist[c] = residual(row, c);
        }
        vector<int> pred(cols, row);

        int scanned = 0, labeled = 0, last = 0;
        int col = -1;

        while (true) {
            if (scanned == labeled) {
                last = scanned;
                auto min = dist[columns[scanned]];
                for (int j = scanned; j < cols; j++) {
                    int c = columns[j];
                    if (dist[c] <= min) {
                        if (dist[c] < min) {
                            min = dist[c];
                            labeled = scanned;
                        }
                        swap(columns[j], columns[labeled++]);
                    }
                }
                for (int j = scanned; j < labeled; j++) {
                    if (int c = columns[j]; col_mate[c] == -1) {
                        col = c;
                        goto done;
                    }
                }
            }
            assert(scanned < labeled);
            int c1 = columns[scanned++];
            int r1 = col_mate[c1];
            for (int j = labeled; j < cols; j++) {
                int c2 = columns[j];
                auto len = residual(r1, c2) - residual(r1, c1);
                assert(len >= 0);
                if (dist[c2] > dist[c1] + len) {
                    dist[c2] = dist[c1] + len;
                    pred[c2] = r1;
                    if (len == 0) {
                        if (col_mate[c2] == -1) {
                            col = c2;
                            goto done;
                        }
                        swap(columns[j], columns[labeled++]);
                    }
                }
            }
        }

    done:;
        for (int i = 0; i < last; i++) {
            int c = columns[i];
            pi[c] += dist[c] - dist[col];
        }

        int t = col;
        while (t != -1) {
            col = t;
            int r = pred[col];
            col_mate[col] = r;
            swap(row_mate[r], t);
        }
    }

    CostSum total = 0;
    for (int u = 0; u < rows; u++) {
        total += cost[u][row_mate[u]];
    }
    return make_pair(total, move(row_mate));
}
