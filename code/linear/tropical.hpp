#pragma once

#include <bits/stdc++.h>
using namespace std;

// Tropical matrix operations (max plus or min plus)
template <typename T>
struct tmat {
    using vec = vector<T>;
    using unit_type = T;
    static constexpr T inf = numeric_limits<T>::lowest() / 2;
    static const T add(const T& a, const T& b) { return max(a, b); }
    static const T mul(const T& a, const T& b) { return a + b; }
    int n, m;
    T* data = nullptr;

    tmat() : n(0), m(0) {}
    tmat(int n, int m, const T& v = inf) { assign(n, m, v); }
    tmat(const tmat& o) : n(o.n), m(o.m), data(new T[n * m]) {
        copy(o.data, o.data + n * m, data);
    }
    tmat(tmat&& o) : tmat() { *this = move(o); }
    tmat& operator=(const tmat& o) { return *this = tmat(o); }
    tmat& operator=(tmat&& o) noexcept {
        using std::swap;
        swap(n, o.n), swap(m, o.m), swap(data, o.data);
        return *this;
    }
    ~tmat() { delete[] data; }

    bool operator==(const tmat& o) const {
        return n == o.n && m == o.m && equal(data, data + n * m, o.data);
    }
    bool operator!=(const tmat& o) const { return !(*this == o); }

    void assign(int n, int m, const T& v) {
        this->n = n, this->m = m, delete[] data, data = new T[n * m];
        std::fill(data, data + n * m, v);
    }

    array<int, 2> size() const { return {n, m}; }
    T* operator[](int x) const { return data + x * m; }
    T& operator[](array<int, 2> xy) const { return data + (xy[0] * m + xy[1]); }

    template <typename U>
    static tmat from(const vector<vector<U>>& vals) {
        int n = vals.size(), m = n ? vals[0].size() : 0;
        tmat a(n, m);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                a[i][j] = vals[i][j];
        return a;
    }

    template <typename O = T>
    friend auto tovec(const tmat& a) {
        vector<vector<O>> m(a.n, vector<T>(a.m));
        for (int i = 0; i < a.n; i++)
            for (int j = 0; j < a.m; j++)
                m[i][j] = a[i][j];
        return m;
    }

    static tmat identity(int n) {
        tmat a(n, n, inf);
        for (int i = 0; i < n; i++)
            a[i][i] = 0;
        return a;
    }

    friend tmat transpose(const tmat& a) {
        tmat b(a.m, a.n);
        for (int i = 0; i < a.m; i++)
            for (int j = 0; j < a.n; j++)
                b[i][j] = a[j][i];
        return b;
    }

    friend tmat& operator+=(tmat& a, const tmat& b) {
        assert(a.size() == b.size() && "Different matrix dimensions");
        for (int i = 0, s = a.n * a.m; i < s; i++)
            a[i] = add(a[i], b[i]);
        return a;
    }

    friend tmat operator*(const tmat& a, const tmat& b) {
        assert(a.m == b.n && "Invalid proper matrix multiplication");
        tmat c(a.n, b.m, inf);
        for (int i = 0; i < a.n; i++)
            for (int k = 0; k < a.m; k++)
                for (int j = 0; j < b.m; j++)
                    c[i][j] = add(c[i][j], mul(a[i][k], b[k][j]));
        return c;
    }

    friend vec operator*(const tmat& a, const vec& b) {
        assert(a.m == int(b.size()) && "Invalid matrix/vector multiplication");
        vec c(a.n, inf);
        for (int i = 0; i < a.n; i++)
            for (int j = 0; j < a.m; j++)
                c[i] = add(c[i], mul(a[i][j], b[j]));
        return c;
    }

    friend tmat operator^(tmat a, int64_t e) {
        assert(a.n == a.m && "Matrix exp operand is not square");
        tmat c = tmat::identity(a.n);
        while (e > 0) {
            if (e & 1)
                c = c * a;
            if (e >>= 1)
                a = a * a;
        }
        return c;
    }

    friend tmat operator+(tmat a, const tmat& b) { return a += b; }
    tmat operator*=(const tmat& b) { return *this = *this * b; }
    tmat operator^=(int64_t e) { return *this = *this ^ e; }

    friend string to_string(const tmat& a) {
        vector<vector<string>> cell(a.n, vector<string>(a.m));
        vector<size_t> w(a.m);
        for (int i = 0; i < a.n; i++) {
            for (int j = 0; j < a.m; j++) {
                using std::to_string;
                if constexpr (std::is_same<T, string>::value) {
                    cell[i][j] = a[i][j];
                } else {
                    cell[i][j] = to_string(a[i][j]);
                }
                w[j] = max(w[j], cell[i][j].size());
            }
        }
        string s;
        for (int i = 0; i < a.n; i++) {
            for (int j = 0; j < a.m; j++) {
                s += string(w[j] + 1 - cell[i][j].size(), ' ');
                s += cell[i][j];
            }
            s += '\n';
        }
        return s;
    }

    friend ostream& operator<<(ostream& out, const tmat& a) {
        return out << to_string(a);
    }
    friend istream& operator>>(istream& in, tmat& a) {
        for (int i = 0; i < a.n * a.m; i++)
            in >> a.data[i];
        return in;
    }
};
