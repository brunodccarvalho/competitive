#pragma once

#include <bits/stdc++.h>
using namespace std;

enum LPState { LP_FEASIBLE = 0, LP_OPTIMAL = 1, LP_UNBOUNDED = 2, LP_IMPOSSIBLE = 3 };
enum LPVarType { PRIMAL = 0, DUAL = 1 };
enum LPVarState { NONBASIC = 0, BASIC = 1 };
using Variable = pair<LPVarType, int>;
using Location = pair<LPVarState, int>;

string to_string(LPState state) {
    static const char* ss[] = {"feasible", "optimal", "unbounded", "impossible"};
    return ss[int(state)];
}

// Maximize Cx  s.t. Ax<=B, x>=0, and maintain A,B,C,basis online
template <typename T = double>
struct simplex {
    int N, M; // N=#primals, M=#duals=#constraints
    vector<vector<T>> A;
    vector<T> B, C;
    vector<Variable> row, col;
    vector<Location> primal, dual;
    T optimum = T(0), eps = T(25) * numeric_limits<T>::epsilon();

    explicit simplex(int n, int m)
        : N(n), M(m), A(m, vector<T>(n)), B(m), C(n), row(m), col(n), primal(n), dual(m) {
        assert(n > 0 && m > 0);
        for (int j = 0; j < N; j++) {
            col[j] = {PRIMAL, j};
            primal[j] = {NONBASIC, j};
        }
        for (int i = 0; i < M; i++) {
            row[i] = {DUAL, i};
            dual[i] = {BASIC, i};
        }
    }

    void dualize() {
        vector<vector<T>> D(N, vector<T>(M));
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                D[j][i] = -A[i][j];
            }
        }
        for (int i = 0; i < M; i++) {
            B[i] = -B[i];
            row[i].first = row[i].first == PRIMAL ? DUAL : PRIMAL;
            dual[i].first = dual[i].first == BASIC ? NONBASIC : BASIC;
        }
        for (int j = 0; j < N; j++) {
            C[j] = -C[j];
            col[j].first = col[j].first == PRIMAL ? DUAL : PRIMAL;
            primal[j].first = primal[j].first == BASIC ? NONBASIC : BASIC;
        }
        optimum = -optimum;
        swap(N, M), swap(A, D), swap(B, C), swap(row, col), swap(primal, dual);
    }

    // Modify the objective row coefficient of primal variable x by +dc. O(n)
    void update_primal_variable(int j, T dc) {
        auto [state, x] = primal[j];
        if (state == NONBASIC) {
            C[x] += dc;
        } else {
            for (int k = 0; k < N; k++) {
                C[k] -= dc * A[x][k];
            }
            optimum += dc * B[x];
        }
    }

    // Modify the constraint bound of dual variable y by +db. O(m)
    void update_dual_variable(int i, T db) {
        auto [state, y] = dual[i];
        if (state == BASIC) {
            B[y] += db;
        } else {
            for (int k = 0; k < M; k++) {
                B[k] += db * A[k][y];
            }
            optimum -= db * C[y];
        }
    }

    // Add a new primal variable/dual constraint with objective c / tableau column a
    // The column vector references the initial tableau's constraints. O(m phi)
    int add_primal_variable(T c, const vector<T>& a) {
        C.push_back(c), col.push_back({PRIMAL, N}), primal.push_back({NONBASIC, N});
        for (int i = 0; i < M; i++) {
            A[i].push_back(0);
        }
        for (int i = 0; i < M; i++) {
            if (auto [state, j] = dual[i]; state == BASIC) {
                A[j][N] += a[i];
            } else if (abs(a[i]) > eps) {
                for (int r = 0; r < M; r++) {
                    A[r][N] += a[i] * A[r][j];
                }
                C[N] += a[i] * C[j];
            }
        }
        return N++;
    }

    // Add a new dual variable / primal constraint with bound b / tableau row a
    // The row vector references the initial tableau's variables. O(n phi)
    int add_dual_variable(T b, const vector<T>& a) {
        B.push_back(b), row.push_back({DUAL, M}), dual.push_back({BASIC, M});
        A.emplace_back(N);
        for (int j = 0; j < N; j++) {
            if (auto [state, i] = primal[j]; state == NONBASIC) {
                A[M][i] += a[j];
            } else if (abs(a[j]) > eps) {
                for (int c = 0; c < N; c++) {
                    A[M][c] -= a[j] * A[i][c];
                }
                B[M] -= a[j] * B[i];
            }
        }
        return M++;
    }

    auto run_dual_primal() {
        if (run_dual_method() == LP_FEASIBLE) {
            return run_primal_method() == LP_FEASIBLE ? LP_OPTIMAL : LP_UNBOUNDED;
        } else {
            return LP_IMPOSSIBLE;
        }
    }

    auto run_primal_dual() {
        if (run_primal_method() == LP_FEASIBLE) {
            return run_dual_method() == LP_FEASIBLE ? LP_OPTIMAL : LP_IMPOSSIBLE;
        } else {
            return LP_UNBOUNDED; // might be impossible actually, don't trust this
        }
    }

    // Extract the basis vector. First N cells are primals, last M cells are duals
    auto extract() const {
        vector<T> X(N + M);
        for (int i = 0; i < M; i++) {
            if (row[i].first == PRIMAL) {
                X[row[i].second] = B[i];
            } else {
                X[row[i].second + N] = B[i];
            }
        }
        return X;
    }

    int potential() const {
        int ans = 0;
        for (int j = 0; N <= M && j < N; j++) { // do this if N<=M
            ans += primal[j].first == BASIC;
        }
        for (int i = 0; N > M && i < M; i++) { // do this if N>M
            ans += dual[i].first == NONBASIC;
        }
        return ans;
    }

    auto get(LPVarType type = PRIMAL, LPVarState state = BASIC) const {
        vector<int> ans;
        for (int j = 0; j < N && type == PRIMAL; j++) {
            if (primal[j].first == state) {
                ans.push_back(j);
            }
        }
        for (int i = 0; i < N && type == DUAL; i++) {
            if (dual[i].first == state) {
                ans.push_back(i);
            }
        }
        return ans;
    }

    bool primal_feasible() const { return *min_element(begin(B), end(B)) >= -eps; }
    bool primal_optimal() const { return *max_element(begin(C), end(C)) <= eps; }

  private:
    vector<int> nonzero;

    auto B_ratio(int r, int c) const { return B[r] / A[r][c]; }
    auto C_ratio(int r, int c) const { return C[c] / A[r][c]; }

    // Bland pivoting rule for tie breaking
    auto B_compare(int i, int r) const { // Remember that B[i] and B[r] are negative here
        return make_pair(B[i], row[i]) < make_pair(B[r], row[r]);
    }
    auto C_compare(int j, int c) const {
        return make_pair(C[c], col[j]) < make_pair(C[j], col[c]);
    }
    auto B_ratio_compare(int i, int r, int c) const {
        return make_pair(B_ratio(i, c), row[i]) < make_pair(B_ratio(r, c), row[r]);
    }
    auto C_ratio_compare(int j, int r, int c) const {
        return make_pair(C_ratio(r, j), col[j]) < make_pair(C_ratio(r, c), col[c]);
    }

    auto run_primal_method() {
        while (true) {
            int r = -1, c = -1;
            // Select primal variable/dual constraint to enter primal basis
            for (int j = 0; j < N; j++) {
                if (eps < C[j] && (c == -1 || C_compare(j, c))) {
                    c = j;
                }
            }
            if (c == -1) {
                return LP_FEASIBLE;
            }
            // Select primal constraint/dual variable to leave primal basis
            for (int i = 0; i < M; i++) {
                if (eps < A[i][c] && (r == -1 || B_ratio_compare(i, r, c))) {
                    r = i;
                }
            }
            if (r == -1) {
                return LP_UNBOUNDED;
            }
            pivot(r, c);
        }
    }

    auto run_dual_method() {
        while (true) {
            int r = -1, c = -1;
            // Select dual variable/primal constraint to enter dual basis
            for (int i = 0; i < M; i++) {
                if (B[i] < -eps && (r == -1 || B_compare(i, r))) {
                    r = i;
                }
            }
            if (r == -1) {
                return LP_FEASIBLE;
            }
            // Select dual constraint/primal variable to leave dual basis
            for (int j = 0; j < N; j++) {
                if (A[r][j] < -eps && (c == -1 || C_ratio_compare(j, r, c))) {
                    c = j;
                }
            }
            if (c == -1) {
                return LP_IMPOSSIBLE;
            }
            pivot(r, c);
        }
    }

    void pivot(int r, int c) {
        // r is row variable index (leaving), c is column variable index (entering)
        auto& [row_type, row_var] = row[r];
        auto& [col_type, col_var] = col[c];
        swap(row_type == PRIMAL ? primal[row_var] : dual[row_var],
             col_type == PRIMAL ? primal[col_var] : dual[col_var]);
        swap(row[r], col[c]);

        T div = T(1) / A[r][c];
        A[r][c] = T(1);

        nonzero.clear();

        // Scale pivot row by div and find nonzero columns
        for (int j = 0; j < N; j++) {
            A[r][j] *= div;
            if (abs(A[r][j]) > eps) {
                nonzero.push_back(j);
            }
        }
        B[r] *= div;

        // Adjust the main tableau. No need to multiply by div here (we already did)
        for (int i = 0; i < M; i++) {
            if (i != r && abs(A[i][c]) > eps) {
                auto mul = A[i][c];
                A[i][c] = T(0);
                for (int j : nonzero) {
                    A[i][j] -= mul * A[r][j];
                }
                B[i] -= mul * B[r];
            }
        }

        // Adjust the objective row. No need to multiply by div here (we already did)
        auto mul = C[c];
        C[c] = T(0);
        for (int j : nonzero) {
            C[j] -= mul * A[r][j];
        }
        optimum += mul * B[r];
    }
};
