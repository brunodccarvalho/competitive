#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Radix sort for distance vectors
 * Useful for shortest path problems and the like
 * The output of the sort functions is a vector of indices into the dist vector
 * such that dist[idx[0]], dist[idx[1]]... is sorted.
 */
inline namespace outline_radix_sort {

template <int B = 6, typename T>
void radix_sort_idx(int* idx, int* cnt, int N, int d, const vector<T>& dist) {
    const int s = B * d, P = 1 << B, mask = P - 1;
    for (int n = 0; n < N; n++) {
        cnt[(dist[n] >> s) & mask]++;
    }
    for (int sum = N, i = P - 1; i >= 0; i--) {
        sum -= cnt[i], cnt[i] = sum;
    }
    for (int n = 0; n < N; n++) {
        idx[cnt[(dist[n] >> s) & mask]++] = n;
    }
}

template <int B = 6, typename T>
void radix_sort_buf(int* idx, int* buf, int* cnt, int N, int d, const vector<T>& dist) {
    const int s = B * d, P = 1 << B, mask = P - 1;
    for (int j = 0; j < N; j++) {
        cnt[(dist[buf[j]] >> s) & mask]++;
    }
    for (int sum = N, i = P - 1; i >= 0; i--) {
        sum -= cnt[i], cnt[i] = sum;
    }
    for (int j = 0; j < N; j++) {
        idx[cnt[(dist[buf[j]] >> s) & mask]++] = buf[j];
    }
}

template <int B = 6, typename T>
void msb_radix_sort_idx_recurse(int* idx, int* buf, int N, int d, int maxd,
                                const vector<T>& dist) {
    constexpr int P = 1 << B;
    if (N <= 1 || d == maxd)
        return;
    if (N <= 20)
        return std::sort(idx, idx + N, [&](int u, int v) { return dist[u] < dist[v]; });

    int cnt[P]{};
    memcpy(buf, idx, N * sizeof(int));
    radix_sort_buf<B>(idx, buf, cnt, N, maxd - d - 1, dist);
    msb_radix_sort_idx_recurse<B>(idx, buf, cnt[0], d + 1, maxd, dist);
    for (int i = 1; i < P; i++) {
        int len = cnt[i] - cnt[i - 1];
        msb_radix_sort_idx_recurse<B>(idx + cnt[i - 1], buf, len, d + 1, maxd, dist);
    }
}

template <int B = 6, typename T>
void msb_radix_sort_idx(vector<int>& idx, const vector<T>& dist) {
    constexpr int P = 1 << B;
    int N = dist.size(), maxd = 0;
    idx.resize(N);

    auto max = *max_element(begin(dist), end(dist));
    while (max > 0)
        maxd++, max >>= B;
    if (maxd == 0)
        return iota(begin(idx), end(idx), 0);

    int cnt[P]{};
    int* buf = new int[N];
    radix_sort_idx<B>(idx.data(), cnt, N, maxd - 1, dist);
    msb_radix_sort_idx_recurse<B>(idx.data(), buf, cnt[0], 1, maxd, dist);
    for (int i = 1; i < P; i++) {
        int len = cnt[i] - cnt[i - 1];
        msb_radix_sort_idx_recurse<B>(idx.data() + cnt[i - 1], buf, len, 1, maxd, dist);
    }
    delete[] buf;
}

template <int B = 6, typename T>
void lsb_radix_sort_idx(vector<int>& idx, const vector<T>& dist) {
    constexpr int P = 1 << B;
    int N = dist.size(), maxd = 0;
    idx.resize(N);

    long max = *max_element(begin(dist), end(dist));
    while (max > 0)
        maxd++, max >>= B;
    if (maxd == 0)
        return iota(begin(idx), end(idx), 0);

    vector<int> buf(N);
    int cnt[P]{};
    radix_sort_idx<B>(idx.data(), cnt, N, 0, dist);
    for (int d = 1; d < maxd; d++) {
        memset(cnt, 0, sizeof(cnt));
        swap(idx, buf);
        radix_sort_buf<B>(idx.data(), buf.data(), cnt, N, d, dist);
    }
}

} // namespace outline_radix_sort

/**
 * Inplace vector radix sort for positive integers (int, long, uint, ulong)
 */
inline namespace inline_radix_sort {

template <int B = 6, typename T>
void radix_sort_run(T* out, T* in, int* cnt, int N, int d) {
    const int s = B * d, P = 1 << B, mask = P - 1;
    for (int n = 0; n < N; n++) {
        cnt[(in[n] >> s) & mask]++;
    }
    for (int sum = N, i = P - 1; i >= 0; i--) {
        sum -= cnt[i], cnt[i] = sum;
    }
    for (int n = 0; n < N; n++) {
        out[cnt[(in[n] >> s) & mask]++] = in[n];
    }
}

template <int B = 6, typename T>
void msb_radix_sort_recurse(T* out, T* in, int N, int d, int maxd) {
    constexpr int P = 1 << B;
    if (N <= 1 || d == maxd)
        return;
    if (N <= 10)
        return std::sort(out, out + N);

    int cnt[P]{};
    memcpy(in, out, N * sizeof(T));
    radix_sort_run<B>(out, in, cnt, N, maxd - d - 1);
    msb_radix_sort_recurse<B>(out, in, cnt[0], d + 1, maxd);
    for (int i = 1; i < P; i++) {
        int len = cnt[i] - cnt[i - 1];
        msb_radix_sort_recurse<B>(out + cnt[i - 1], in, len, d + 1, maxd);
    }
}

template <int B = 6, typename T>
void msb_radix_sort(vector<T>& v) {
    constexpr int P = 1 << B;
    int N = v.size(), maxd = 0;
    if (N <= 1)
        return;
    if (N <= 30)
        return sort(begin(v), end(v));

    auto max = *max_element(begin(v), end(v));
    while (max > 0)
        maxd++, max >>= B;
    if (maxd == 0)
        return;

    int cnt[P]{};
    vector<T> buf = v;
    radix_sort_run<B>(v.data(), buf.data(), cnt, N, maxd - 1);
    msb_radix_sort_recurse<B>(v.data(), buf.data(), cnt[0], 1, maxd);
    for (int i = 1; i < P; i++) {
        int len = cnt[i] - cnt[i - 1];
        msb_radix_sort_recurse<B>(v.data() + cnt[i - 1], buf.data(), len, 1, maxd);
    }
}

template <int B = 6, typename T>
void lsb_radix_sort(vector<T>& v) {
    constexpr int P = 1 << B;
    int N = v.size(), maxd = 0;
    if (N <= 1)
        return;
    if (N <= 30)
        return sort(begin(v), end(v));

    auto max = *max_element(begin(v), end(v));
    while (max > 0)
        maxd++, max >>= B;
    if (maxd == 0)
        return;

    int cnt[P]{};
    vector<T> buf = v;
    radix_sort_run<B>(v.data(), buf.data(), cnt, N, 0);
    for (int d = 1; d < maxd; d++) {
        memset(cnt, 0, sizeof(cnt));
        swap(v, buf);
        radix_sort_run<B>(v.data(), buf.data(), cnt, N, d);
    }
}

} // namespace inline_radix_sort
