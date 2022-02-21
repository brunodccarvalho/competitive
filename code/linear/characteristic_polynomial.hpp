#pragma once

#include "linear/matrix.hpp"
#include "numeric/polynomial.hpp"

/**
 * Source: https://github.com/Aeren1564/Algorithms/.../characteristic_polynomial
 */
template <typename T>
vector<T> characteristic_polynomial(const mat<T>& a, bool flip = false) {
    using namespace polymath;
    assert(a.n == a.m);
    int n = a.n;
    for (int j = 0; j < n; j++) {
        int pivot = -1;
        for (int i = j + 1; i < n; ++i) {
            if (a[i][j]) {
                pivot = i;
                break;
            }
        }
        if (pivot == -1)
            continue;
        for (int k = 0; pivot != j + 1 && k < n; ++k) {
            swap(a[j + 1][k], a[pivot][k]);
        }
        for (int k = 0; pivot != j + 1 && k < n; ++k) {
            swap(a[k][j + 1], a[k][pivot]);
        }
        for (int i = j + 2; i < n; ++i) {
            if (a[i][j]) {
                T t = a[i][j] / a[j + 1][j];
                for (int k = 0; k < n; ++k)
                    a[i][k] -= a[j + 1][k] * t;
                for (int k = 0; k < n; ++k)
                    a[k][j + 1] += a[k][i] * t;
            }
        }
    }
    vector<T> X, Y;
    for (int x = 0; x <= n; x++) {
        mat<T> b(n, n);
        for (auto i = 0; i < n; ++i)
            for (auto j = 0; j < n; ++j)
                b[i][j] = i == j ? x - a[i][j] : -a[i][j];

        T sign = 1;
        for (auto j = 0; j < n - 1; ++j) {
            if (b[j + 1][j]) {
                for (auto k = 0; k < n; ++k)
                    swap(b[j][k], b[j + 1][k]);
                sign = -sign;
            }
            if (b[j][j] && b[j + 1][j]) {
                T t = b[j + 1][j] / b[j][j];
                for (auto jj = j; jj < n; ++jj)
                    b[j + 1][jj] -= b[j][jj] * t;
            }
        }
        T y = sign;
        for (auto i = 0; i < n; ++i)
            y *= b[i][i];
        X.push_back(x), Y.push_back(y);
    }

    return polymath::interpolate(X, Y) * T(flip ? -1 : 1);
}
