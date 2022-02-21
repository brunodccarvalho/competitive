#pragma once

#include "numeric/fft.hpp"
#include "numeric/polynomial.hpp"
#include "linear/berlekamp_massey.hpp"

// ~ Black box linear algebra
// These are matrix oracles for matrix-vector multiplcation (square operators only)

/**
 * Non-zero entries given as a list
 * Complexity: O(m) M-V product
 */
template <typename T>
struct sparse_matrix {
    static constexpr bool PRODUCT = false;
    int n;
    vector<tuple<int, int, T>> data;

    explicit sparse_matrix(int n) : n(n) {}
    sparse_matrix(int n, vector<tuple<int, int, T>> data) : n(n), data(move(data)) {}

    template <typename U>
    sparse_matrix(int n, const vector<array<int, 2>>& pos, const vector<U>& input)
        : n(n), data(pos.size()) {
        for (int s = pos.size(), i = 0; i < s; i++)
            data[i] = {pos[i][0], pos[i][1], input[i]};
    }

    int size() const { return n; }

    auto operator*(const vector<T>& v) const {
        vector<T> ans(n);
        for (auto [i, j, u] : data)
            ans[i] += u * v[j];
        return ans;
    }
};

/**
 * Non-zero entries only on the diagonal
 * Complexity: O(n) M-V product
 */
template <typename T>
struct diagonal_matrix {
    static constexpr bool PRODUCT = true;
    vector<T> diag;

    explicit diagonal_matrix(int n) : diag(n) {}
    diagonal_matrix(vector<T> data) : diag(move(data)) {}

    template <typename U>
    diagonal_matrix(const vector<U>& input) : diag(input.size()) {
        copy(begin(input), end(input), begin(diag));
    }

    int size() const { return diag.size(); }

    auto operator*(diagonal_matrix mat) const {
        for (int i = 0, n = size(); i < n; i++)
            mat.diag[i] *= diag[i];
        return mat;
    }

    auto operator*(vector<T> v) const {
        for (int i = 0, n = size(); i < n; i++)
            v[i] *= diag[i];
        return move(v);
    }
};

/**
 * Any dense matrix
 * Complexity: O(n^2) M-V product
 */
template <typename T>
struct dense_matrix {
    static constexpr bool PRODUCT = true;
    int n;
    vector<T> data;

    explicit dense_matrix(int n) : n(n), data(n * n) {}
    dense_matrix(int n, vector<T> data) : n(n), data(move(data)) {}

    template <typename U>
    dense_matrix(int n, const vector<U>& input) : data(input.size()) {
        copy(begin(input), end(input), begin(data));
    }

    int size() const { return n; }

    auto& operator[](int i) const { return data.data() + i * n; }

    auto operator*(const dense_matrix& mat) const {
        dense_matrix ans(n);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                for (int k = 0; k < n; k++)
                    ans[i][j] += (*this)[i][k] * mat[k][j];
        return ans;
    }

    auto operator*(const vector<T>& v) const {
        vector<T> ans(n);
        for (int i = 0, k = 0; i < n; i++)
            for (int j = 0; j < n; j++, k++)
                ans[i] += data[k] * v[j];
        return ans;
    }
};

/**
 * Circulant matrices, first row is some polynomial P, every row after is shifted
 * to the right by one position. The constructor's polynomial is the first row.
 * Complexity: O(n log n) M-V product
 */
template <typename T>
struct circulant_matrix {
    static constexpr bool PRODUCT = false;
    vector<T> poly;

    explicit circulant_matrix(int n) : poly(2 * n) {}
    circulant_matrix(int n, const vector<T>& input) : poly(2 * n) {
        copy(begin(input), end(input), begin(poly));
        copy(begin(input), end(input), begin(poly) + n);
    }

    template <typename U>
    circulant_matrix(int n, const vector<U>& input) : poly(2 * n) {
        copy(begin(input), end(input), begin(poly));
        copy(begin(input), end(input), begin(poly) + n);
    }

    int size() const { return poly.size() / 2; }

    auto operator*(vector<T> v) const {
        reverse(begin(v), end(v));
        vector<T> cyclic_conv = fft::fft_multiply(poly, v), ans(size());
        for (int i = 0, n = size(); i < n; i++)
            ans[i] = cyclic_conv[2 * n - i - 1];
        return ans;
    }
};

/**
 * Any matrix with exactly one 1 on each row, and the rest zeros
 * Complexity: O(n) M-V product
 */
template <typename T>
struct permutation_matrix {
    static constexpr bool PRODUCT = false;
    vector<int> perm;

    explicit permutation_matrix(int n) : perm(n) {}
    permutation_matrix(const vector<int>& perm) : perm(perm) {}

    int size() const { return perm.size(); }

    auto operator*(const vector<T>& v) const {
        vector<T> ans(size());
        for (int i = 0, n = size(); i < n; i++)
            ans[i] = v[perm[i]];
        return ans;
    }
};

/**
 * Matrix resulting from the outer product of two vectors or polynomials a and b.
 *        mat[i][j] = a[i] * b[j]
 * Complexity: O(n) M-V product
 */
template <typename T>
struct outer_product_matrix {
    static constexpr bool PRODUCT = false;
    vector<T> a, b;

    explicit outer_product_matrix(int n) : a(n), b(n) {}
    outer_product_matrix(vector<T> a, vector<T> b) : a(move(a)), b(move(b)) {}

    template <typename U>
    outer_product_matrix(const vector<U>& ain, const vector<U>& bin)
        : a(ain.size()), b(ain.size()) {
        copy(begin(ain), end(ain), begin(a));
        copy(begin(bin), end(bin), begin(b));
    }

    int size() const { return a.size(); }

    auto operator*(const vector<T>& v) const {
        T inner = 0;
        vector<T> ans(size());
        for (int i = 0, n = size(); i < n; i++)
            inner += b[i] * v[i];
        for (int i = 0, n = size(); i < n; i++)
            ans[i] = a[i] * inner;
        return ans;
    }
};

/**
 * A^n = P[0]*A^n-1 + P[1]*A^n-2 + ... + P[n-2]*A + P[n-1]*I
 * You might want to reverse P for practical use
 * Complexity: O(n(n + 2P(n)))
 */
template <typename T, typename Matrix>
auto matrix_minimal_polynomial(const Matrix& A) {
    // Compute y A^k x for k=0...2n+security then run Berlekamp-Massey
    using namespace polymath;
    int n = A.size();
    vector<T> x = rand_poly<T>(n), y = rand_poly<T>(n), a(2 * n + 3);

    for (int i = 0; i <= 2 * n + 2; i++) {
        a[i] = inner_product(x, y);
        x = A * x;
    }

    return berlekamp_massey(a);
}

/**
 * Just read off the determinant of A for its minimal polynomial
 * Complexity: O(n(n + 2P(n)))
 */
template <typename T, typename Matrix>
auto matrix_determinant(const Matrix& A) {
    // Extract the coefficient of x^0 in the minimal polynomial, careful of signs
    auto M = matrix_minimal_polynomial<T>(A);
    int d = M.size(), n = A.size();
    return d < n ? T(0) : (n % 2) ? M[n - 1] : -M[n - 1];
}
