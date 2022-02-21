#pragma once

#include "linear/matrix.hpp"

// ***** Gauss with modnum/fields

// Compute inverse of square matrix, T must be invertible. Returns nullopt if det=0
template <typename T>
optional<mat<T>> inverse(mat<T> a) {
    assert(a.n == a.m && "Matrix inverse operand is not square");
    auto b = mat<T>::identity(a.n);
    for (int j = 0, i; j < a.n; j++) {
        for (i = j; i < a.n; i++) {
            if (a[i][j]) {
                break;
            }
        }
        if (i == a.n) {
            return std::nullopt;
        }
        if (i != j) {
            for (int k = 0; k < a.n; k++) {
                swap(a[i][k], a[j][k]);
                swap(b[i][k], b[j][k]);
            }
        }
        T div = T(1) / a[j][j];
        for (int k = 0; k < a.n; k++) {
            b[j][k] *= div;
        }
        for (int k = a.n - 1; k >= j; k--) {
            a[j][k] *= div;
        }
        for (i = j + 1; i < a.n; i++) {
            if (a[i][j]) {
                for (int k = 0; k < a.n; k++) {
                    b[i][k] -= a[i][j] * b[j][k];
                }
                for (int k = a.n - 1; k >= j; k--) {
                    a[i][k] -= a[i][j] * a[j][k];
                }
            }
        }
    }
    for (int j = a.n - 1; j >= 0; j--)
        for (int i = 0; i < j; i++)
            for (int k = 0; k < a.n; k++)
                b[i][k] -= a[i][j] * b[j][k];
    return b;
}

// Solve any linear system, T must be a field. Returns empty vector if no solution.
template <typename T>
auto solve_linear_system(mat<T> a, vector<T> b) {
    assert(a.n == int(b.size()));
    int r = 0, c = 0;
    vector<int> cols, nonzero;
    for (int i; r < a.n && c < a.m; c++) {
        for (i = r; i < a.n; i++) {
            if (a[i][c]) {
                break;
            }
        }
        if (i == a.n) {
            continue;
        }
        if (i != r) {
            swap(b[i], b[r]);
            for (int k = 0; k < a.m; k++) {
                swap(a[i][k], a[r][k]);
            }
        }
        T div = T(1) / a[r][c];
        b[r] *= div;
        for (int k = a.m - 1; k >= c; k--) {
            if (a[r][k]) {
                a[r][k] *= div;
                nonzero.push_back(k);
            }
        }
        for (i = r + 1; i < a.n; i++) {
            if (a[i][c]) {
                b[i] -= a[i][c] * b[r];
                for (int k : nonzero) {
                    a[i][k] -= a[i][c] * a[r][k];
                }
            }
        }
        cols.push_back(c), nonzero.clear(), r++;
    }
    // Verify system is indeed solved
    for (int i = r; i < a.n; i++) {
        if (b[i]) {
            return vector<T>();
        }
    }
    // Complete the row reduction, but only for relevant columns
    for (int i = r - 1; i > 0; i--) {
        for (int y = r - 1; y >= i; y--) {
            if (a[i][cols[y]]) {
                nonzero.push_back(cols[y]);
            }
        }
        for (int k = i - 1; k >= 0; k--) {
            if (a[k][cols[i]]) {
                b[k] -= a[k][cols[i]] * b[i];
                for (int j : nonzero) {
                    a[k][j] -= a[k][cols[i]] * a[i][j];
                }
            }
        }
        nonzero.clear();
    }
    vector<T> ans(a.m);
    for (int i = r - 1; i >= 0; i--) {
        ans[cols[i]] = b[i];
    }
    return ans;
}

// Solve any linear system and get solution space basis. Returns {sol, sol space basis}
template <typename T>
auto solve_linear_system_basis(mat<T> a, vector<T> b) {
    assert(a.n == int(b.size()));
    int r = 0, c = 0;
    vector<int> cols, other, nonzero;
    for (int i; r < a.n && c < a.m; c++) {
        for (i = r; i < a.n; i++) {
            if (a[i][c]) {
                break;
            }
        }
        if (i == a.n) {
            other.push_back(c);
            continue;
        }
        if (i != r) {
            swap(b[i], b[r]);
            for (int k = 0; k < a.m; k++) {
                swap(a[i][k], a[r][k]);
            }
        }
        T div = T(1) / a[r][c];
        b[r] *= div;
        for (int k = a.m - 1; k >= c; k--) {
            if (a[r][k]) {
                a[r][k] *= div;
                nonzero.push_back(k);
            }
        }
        for (i = r + 1; i < a.n; i++) {
            if (a[i][c]) {
                b[i] -= a[i][c] * b[r];
                for (int k : nonzero) {
                    a[i][k] -= a[i][c] * a[r][k];
                }
            }
        }
        cols.push_back(c), nonzero.clear(), r++;
    }
    // Verify system is indeed solved
    for (int i = r; i < a.n; i++) {
        if (b[i]) {
            return make_pair(vector<T>(), vector<vector<T>>());
        }
    }
    // Remaining solution basis columns
    while (c < a.m) {
        other.push_back(c++);
    }
    // Complete the row reduction, including other columns
    for (int i = r - 1; i > 0; i--) {
        for (int j = a.m - 1; j >= cols[i]; j--) {
            if (a[i][j]) {
                nonzero.push_back(j);
            }
        }
        for (int k = i - 1; k >= 0; k--) {
            if (a[k][cols[i]]) {
                b[k] -= a[k][cols[i]] * b[i];
                for (int j : nonzero) {
                    a[k][j] -= a[k][cols[i]] * a[i][j];
                }
            }
        }
        nonzero.clear();
    }
    // One solution of the system
    vector<T> ans(a.m);
    for (int i = 0; i < r; i++) {
        ans[cols[i]] = b[i];
    }
    // The space of solutions, possibly empty
    int rank = other.size();
    vector<vector<T>> basis(rank);
    for (int p = 0; p < rank; p++) {
        basis[p].resize(a.m);
        basis[p][other[p]] = -1;
    }
    // For basis[p], set the column other[p] to -1, then for each of the A basis rows
    // a[0], ..., a[r-1] compute the coefficient for column cols[0], ..., cols[r-1]
    for (int i = 0; i < r; i++) {
        for (int p = 0; p < rank; p++) {
            basis[p][cols[i]] = a[i][other[p]];
        }
    }
    return make_pair(move(ans), move(basis));
}

// Row reduction inplace to compute rank and determinant, T must be a field.
template <typename T>
auto gauss_elimination(mat<T>& a, bool full = true) {
    int r = 0, c = 0;
    T determinant = 1;
    vector<int> nonzero, cols;
    for (int i; r < a.n && c < a.m; c++) {
        for (i = r; i < a.n; i++) {
            if (a[i][c]) {
                break;
            }
        }
        if (i == a.n) {
            determinant = 0;
            continue;
        }
        if (i != r) {
            for (int k = 0; k < a.m; k++) {
                swap(a[i][k], a[r][k]);
            }
            determinant = -determinant;
        }
        T div = T(1) / a[r][c];
        determinant *= a[r][c];
        for (int k = a.m - 1; k >= c; k--) {
            if (a[r][k]) {
                a[r][k] *= div;
                nonzero.push_back(k);
            }
        }
        for (i = r + 1; i < a.n; i++) {
            if (a[i][c]) {
                for (int k : nonzero) {
                    a[i][k] -= a[i][c] * a[r][k];
                }
            }
        }
        nonzero.clear(), r++;
        cols.push_back(c);
    }
    // Complete the row reduction, including other columns
    for (int i = r - 1; full && i > 0; i--) {
        for (int j = a.m - 1; j >= cols[i]; j--) {
            if (a[i][j]) {
                nonzero.push_back(j);
            }
        }
        for (int k = i - 1; k >= 0; k--) {
            if (a[k][cols[i]]) {
                for (int j : nonzero) {
                    a[k][j] -= a[k][cols[i]] * a[i][j];
                }
            }
        }
        nonzero.clear();
    }
    return make_tuple(determinant, r, move(cols));
}

// ***** Gauss with floating point

// Compute inverse of square matrix. Returns nullopt if det=0
template <typename D>
optional<mat<D>> float_inverse(mat<D> a) {
    constexpr D eps = 1e2 * numeric_limits<D>::epsilon();
    assert(a.n == a.m && "Matrix inverse operand is not square");
    int n = a.n;
    auto b = mat<D>::identity(n);
    for (int j = 0, i, x; j < n; j++) {
        for (i = x = j; x < n; x++) {
            if (abs(a[i][j]) < abs(a[x][j])) {
                i = x;
            }
        }
        if (i == n || abs(a[i][j] <= eps)) {
            return std::nullopt;
        }
        if (i != j) {
            for (int k = 0; k < n; k++) {
                swap(a[i][k], a[j][k]);
                swap(b[i][k], b[j][k]);
            }
        }
        for (int k = 0; k < n; k++)
            b[j][k] /= a[j][j];
        for (int k = n - 1; k >= j; k--)
            a[j][k] /= a[j][j];
        for (i = j + 1; i < n; i++) {
            for (int k = 0; k < n; k++)
                b[i][k] -= a[i][j] * b[j][k];
            for (int k = n - 1; k >= j; k--)
                a[i][k] -= a[i][j] * a[j][k];
        }
    }
    for (int j = n - 1; j >= 0; j--)
        for (int i = 0; i < j; i++)
            for (int k = 0; k < n; k++)
                b[i][k] -= a[i][j] * b[j][k];
    return b;
}

// Solve any linear system. Returns empty vector for no solution.
template <typename D>
auto solve_float_linear_system(mat<D> a, vector<D> b, bool trim = true) {
    constexpr D eps = 1e2 * numeric_limits<D>::epsilon();
    assert(a.n == int(b.size()));
    int r = 0, c = 0;
    vector<int> cols, nonzero;
    for (int i, x; r < a.n && c < a.m; c++) {
        for (i = x = r; x < a.n; x++) {
            if (abs(a[i][c]) < abs(a[x][c])) {
                i = x;
            }
        }
        if (i == a.n || abs(a[i][c]) <= eps) {
            continue;
        }
        if (i != r) {
            swap(b[i], b[r]);
            for (int k = 0; k < a.m; k++) {
                swap(a[i][k], a[r][k]);
            }
        }
        b[r] /= a[r][c];
        for (int k = a.m - 1; k >= c; k--) {
            a[r][k] /= a[r][c];
            if (!trim || abs(a[r][k]) > eps) {
                nonzero.push_back(k);
            }
        }
        for (i = r + 1; i < a.n; i++) {
            b[i] -= a[i][c] * b[r];
            for (int k : nonzero) {
                a[i][k] -= a[i][c] * a[r][k];
            }
        }
        cols.push_back(c), nonzero.clear(), r++;
    }
    // Verify system is indeed solved
    for (int i = r; i < a.n; i++) {
        if (abs(b[i]) > eps) {
            return vector<D>();
        }
    }
    // Complete the row reduction
    for (int i = r - 1; i > 0; i--) {
        for (int y = r - 1; y >= i; y--) {
            if (!trim || abs(a[i][cols[y]]) > eps) {
                nonzero.push_back(cols[y]);
            }
        }
        for (int k = i - 1; k >= 0; k--) {
            b[k] -= a[k][cols[i]] * b[i];
            for (int j : nonzero) {
                a[k][j] -= a[k][cols[i]] * a[i][j];
            }
        }
        nonzero.clear();
    }
    vector<D> ans(a.m);
    for (int i = r - 1; i >= 0; i--) {
        ans[cols[i]] = b[i];
    }
    return ans;
}

// Solve any linear system and get solution space basis. Returns {sol, sol space basis}
template <typename D>
auto solve_float_linear_system_basis(mat<D> a, vector<D> b, bool trim = true) {
    constexpr D eps = 1e2 * numeric_limits<D>::epsilon();
    assert(a.n == int(b.size()));
    int r = 0, c = 0;
    vector<int> cols, other, nonzero;
    for (int i, x; r < a.n && c < a.m; c++) {
        for (i = x = r; i < a.n; i++) {
            if (abs(a[i][c]) < abs(a[x][c])) {
                i = x;
            }
        }
        if (i == a.n || abs(a[i][c]) <= eps) {
            other.push_back(c);
            continue;
        }
        if (i != r) {
            swap(b[i], b[r]);
            for (int k = 0; k < a.m; k++) {
                swap(a[i][k], a[r][k]);
            }
        }
        b[r] /= a[r][c];
        for (int k = a.m - 1; k >= c; k--) {
            a[r][k] /= a[r][c];
            if (!trim || abs(a[r][k]) > eps) {
                nonzero.push_back(k);
            }
        }
        for (i = r + 1; i < a.n; i++) {
            b[i] -= a[i][c] * b[r];
            for (int k : nonzero) {
                a[i][k] -= a[i][c] * a[r][k];
            }
        }
        cols.push_back(c), nonzero.clear(), r++;
    }
    // Verify system is indeed solved
    for (int i = r; i < a.n; i++) {
        if (abs(b[i]) > eps) {
            return make_pair(vector<D>(), vector<vector<D>>());
        }
    }
    // Remaining solution basis columns
    while (c < a.m) {
        other.push_back(c++);
    }
    // Complete the row reduction, including other columns
    for (int i = r - 1; i > 0; i--) {
        for (int j = a.m - 1; j >= cols[i]; j--) {
            if (!trim || abs(a[i][j]) > eps) {
                nonzero.push_back(j);
            }
        }
        for (int k = i - 1; k >= 0; k--) {
            b[k] -= a[k][cols[i]] * b[i];
            for (int j : nonzero) {
                a[k][j] -= a[k][cols[i]] * a[i][j];
            }
        }
        nonzero.clear();
    }
    // One solution of the system
    vector<D> ans(a.m);
    for (int i = 0; i < r; i++) {
        ans[cols[i]] = b[i];
    }
    // The space of solutions, possibly empty
    int rank = other.size();
    vector<vector<D>> basis(rank);
    for (int p = 0; p < rank; p++) {
        basis[p].resize(a.m);
        basis[p][other[p]] = -1;
    }
    // For basis[p], set the column other[p] to -1, then for each of the A basis rows
    // a[0], ..., a[r-1] compute the coefficient for column cols[0], ..., cols[r-1]
    for (int i = 0; i < r; i++) {
        for (int p = 0; p < rank; p++) {
            basis[p][cols[i]] = a[i][other[p]];
        }
    }
    return make_pair(move(ans), move(basis));
}

// Row reduction inplace with partial pivoting to compute rank and determinant.
template <typename D>
auto float_gauss_elimination(mat<D>& a, bool full = true, bool trim = true) {
    constexpr D eps = 1e2 * numeric_limits<D>::epsilon();
    int r = 0, c = 0;
    D determinant = 1;
    vector<int> nonzero, cols;
    for (int i, x; r < a.n && c < a.m; c++) {
        for (i = x = r; x < a.n; x++) {
            if (abs(a[i][c]) < abs(a[x][c])) {
                i = x;
            }
        }
        if (i == a.n || abs(a[i][c]) <= eps) {
            determinant = 0;
            continue;
        }
        if (i != r) {
            for (int k = 0; k < a.m; k++) {
                swap(a[i][k], a[r][k]);
            }
            determinant = -determinant;
        }
        determinant *= a[r][c];
        for (int k = a.m - 1; k >= c; k--) {
            a[r][k] /= a[r][c];
            if (!trim || abs(a[r][k]) > eps) {
                nonzero.push_back(k);
            }
        }
        for (i = r + 1; i < a.n; i++) {
            for (int k : nonzero) {
                a[i][k] -= a[i][c] * a[r][k];
            }
        }
        nonzero.clear(), r++;
        cols.push_back(c);
    }
    // Complete the row reduction, including other columns
    for (int i = r - 1; i > 0; i--) {
        for (int j = a.m - 1; j >= cols[i]; j--) {
            if (!trim || abs(a[i][j]) > eps) {
                nonzero.push_back(j);
            }
        }
        for (int k = i - 1; k >= 0; k--) {
            for (int j : nonzero) {
                a[k][j] -= a[k][cols[i]] * a[i][j];
            }
        }
        nonzero.clear();
    }
    return make_tuple(determinant, r, move(cols));
}
