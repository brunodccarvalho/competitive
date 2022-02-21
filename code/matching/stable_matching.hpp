#pragma once

#include <bits/stdc++.h>
using namespace std;

auto stable_matching(const vector<vector<int>>& u, const vector<vector<int>>& v) {
    int N = u.size();
    vector<int> mu(N, -1), mv(N, -1);
    for (int i = 0; i < N; i++) {
        int k = 0;
        while (mu[i] < 0) {
            int w = u[i][k++];
            int m = mv[w];
            if (m == -1) {
                mu[i] = w;
                mv[w] = i;
            } else if (v[w][i] < v[w][m]) {
                mu[m] = -1;
                mu[i] = w;
                mv[w] = i;
                i = m;
            }
        }
    }
    return make_pair(move(mu), move(mv));
}
