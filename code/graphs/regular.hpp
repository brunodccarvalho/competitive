#pragma once

#include "hash.hpp"
#include "random.hpp"

using edges_t = vector<array<int, 2>>;

edges_t degree_sample(int n, const vector<int>& k) {
    assert(k.size() == unsigned(n));
    int d = accumulate(begin(k), end(k), 0);
    assert(d % 2 == 0);

    vector<int> su(d);
    for (int l = 0, i = 0; i < n; i++)
        for (int j = 0; j < k[i]; j++, l++)
            su[l] = i;

    unordered_set<array<int, 2>> seen;
    seen.reserve(d / 2);
    int restarts = 0;
    do {
        seen.clear();
        vector<int> stub = su;
        int bad = 0, S = d;

        while (S > 0 && bad < 20) {
            int A = 0;
            shuffle(begin(stub), end(stub), mt);

            for (int i = 0; i < S; i += 2) {
                auto [u, v] = minmax({stub[i], stub[i + 1]});
                if (u != v && !seen.count({u, v})) {
                    seen.insert({u, v});
                    swap(stub[i + 0], stub[S - 2]);
                    swap(stub[i + 1], stub[S - 1]);
                    stub.pop_back(), stub.pop_back();
                    A++, i -= 2, S -= 2;
                }
            }

            bad = A ? 0 : bad + S;
        }

        if (S == 0) {
            return edges_t(begin(seen), end(seen));
        }
    } while (restarts++ < 500);

    throw runtime_error("Failed to generate regular sample after 500 restarts");
}

edges_t degree_bipartite_sample(int n, int m, const vector<int>& k,
                                const vector<int>& p) {
    assert(k.size() == unsigned(n) && p.size() == unsigned(m));
    int d = accumulate(begin(k), end(k), 0);
    assert(d == accumulate(begin(p), end(p), 0));

    vector<int> su(d), sv(d);
    for (int l = 0, i = 0; i < n; i++)
        for (int j = 0; j < k[i]; j++, l++)
            su[l] = i;
    for (int l = 0, i = 0; i < m; i++)
        for (int j = 0; j < p[i]; j++, l++)
            sv[l] = i;

    unordered_set<array<int, 2>> seen;
    seen.reserve(d);
    int restarts = 0;
    do {
        seen.clear();
        vector<int> stub[2] = {su, sv};
        int bad = 0, S = d;

        while (S > 0 && bad < 20) {
            int A = 0;
            shuffle(begin(stub[0]), end(stub[0]), mt);
            shuffle(begin(stub[1]), end(stub[1]), mt);

            for (int i = 0; i < S; i++) {
                int u = stub[0][i], v = stub[1][i];
                if (!seen.count({u, v})) {
                    seen.insert({u, v});
                    swap(stub[0][i], stub[0][S - 1]);
                    swap(stub[1][i], stub[1][S - 1]);
                    stub[0].pop_back(), stub[1].pop_back();
                    A++, i--, S--;
                }
            }

            bad = A ? 0 : bad + 1;
        }

        if (S == 0) {
            return edges_t(begin(seen), end(seen));
        }
    } while (restarts++ < 500);

    throw runtime_error("Failed to generate regular sample after 500 restarts");
}

edges_t degree_directed_sample(int n, const vector<int>& out, const vector<int>& in) {
    assert(out.size() == unsigned(n) && in.size() == unsigned(n));
    int d = accumulate(begin(out), end(out), 0);
    assert(d == accumulate(begin(in), end(in), 0));

    vector<int> su(d), sv(d);
    for (int l = 0, i = 0; i < n; i++)
        for (int j = 0; j < out[i]; j++, l++)
            su[l] = i;
    for (int l = 0, i = 0; i < n; i++)
        for (int j = 0; j < in[i]; j++, l++)
            sv[l] = i;

    unordered_set<array<int, 2>> seen;
    seen.reserve(d);
    int restarts = 0;
    do {
        seen.clear();
        vector<int> stub[2] = {su, sv};
        int bad = 0, S = d;

        while (S > 0 && bad < 20) {
            int A = 0;
            shuffle(begin(stub[0]), end(stub[0]), mt);
            shuffle(begin(stub[1]), end(stub[1]), mt);

            for (int i = 0; i < S; i++) {
                int u = stub[0][i], v = stub[1][i];
                if (u != v && !seen.count({u, v})) {
                    seen.insert({u, v});
                    swap(stub[0][i], stub[0][S - 1]);
                    swap(stub[1][i], stub[1][S - 1]);
                    stub[0].pop_back(), stub[1].pop_back();
                    A++, i--, S--;
                }
            }

            bad = A ? 0 : bad + 1;
        }

        if (S == 0) {
            return edges_t(begin(seen), end(seen));
        }
    } while (restarts++ < 500);

    throw runtime_error("Failed to generate regular sample after 500 restarts");
}

edges_t regular_sample(int n, int k) {
    assert(2 <= k && k < n && n * k % 2 == 0);
    vector<int> R(n, k);
    return degree_sample(n, R);
}

edges_t regular_bipartite_sample(int n, int m, int k) {
    assert(0 <= k && k <= m && n * k % m == 0);
    vector<int> L(n, k), R(m, n * k / m);
    return degree_bipartite_sample(n, m, L, R);
}

edges_t regular_directed_sample(int n, int k) {
    assert(1 <= k && k < n);
    vector<int> R(n, k);
    return degree_directed_sample(n, R, R);
}
