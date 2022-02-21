#pragma once

#include <bits/stdc++.h>
using namespace std;

// Compress vector u into type O with numbers 0,1,... handles ties. O(n log n)
template <typename O, typename T>
auto compress(const vector<T>& u, T s = 0) {
    int N = u.size();
    if (N == 0)
        return vector<O>();

    vector<pair<T, int>> ps(N);
    for (int i = 0; i < N; i++) {
        ps[i] = {u[i], i};
    }
    sort(begin(ps), end(ps));

    vector<O> v(N);
    v[ps[0].second] = s;
    for (int i = 1; i < N; i++) {
        v[ps[i].second] = v[ps[i - 1].second] + (ps[i].first != ps[i - 1].first);
    }
    return v;
}
