#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * E[i] = min{j<i} D[j] + cost(j,i)
 * where D[i] is a function of E[i] and cost(j,i) is concave:
 *  cost(j,a) - cost(j,b) >= cost(i,a) - cost(i,b)
 *  cost(i,b) - cost(j,b) >= cost(i,a) - cost(j,a) for j < i < b < a. O(N log N)
 */
template <typename V, typename Dn, typename Cn>
auto solve_1d1d_concave(int N, V zero, Dn&& dn, Cn&& costfn) {
    vector<V> E(N), D(N);
    E[0] = zero;
    D[0] = dn(E[0], 0);

    vector<array<int, 2>> stk;

    auto cost = [&](int j, int i) { return D[j] + costfn(j, i); };

    auto improv = [&](int j, int i, int t) {
        int l = i - 1, r = t;
        while (l + 1 < r) {
            int m = (l + r) / 2;
            cost(j, m) >= cost(i - 1, m) ? l = m : r = m;
        }
        return l;
    };

    for (int i = 1, S = -1; i < N; i++) {
        while (S >= 0) {
            auto [j, t] = stk[S];
            if (cost(i - 1, t) <= cost(j, t)) {
                stk.pop_back(), S--;
            } else {
                break;
            }
        }

        if (int t = S < 0 ? N - 1 : improv(stk[S][0], i, stk[S][1]); t >= i) {
            stk.push_back({i - 1, t}), S++;
        }

        E[i] = cost(stk[S][0], i);
        D[i] = dn(E[i], i);

        if (S >= 0 && stk[S][1] == i) {
            stk.pop_back(), S--;
        }
    }

    return E;
}

/**
 * E[i] = min{j<i} D[j] + cost(j,i)
 * where D[i] is a function of E[i] and cost(j,i) is convex:
 *  cost(j,a) - cost(j,b) <= cost(i,a) - cost(i,b)
 *  cost(i,b) - cost(j,b) <= cost(i,a) - cost(j,a) for j < i < b < a. O(N log N)
 */
template <typename V, typename Dn, typename Cn>
auto solve_1d1d_convex(int N, V zero, Dn&& dn, Cn&& costfn) {
    vector<V> E(N), D(N);
    E[0] = zero;
    D[0] = dn(E[0], 0);

    vector<array<int, 2>> deq;

    auto cost = [&](int j, int i) { return D[j] + costfn(j, i); };

    auto improv = [&](int j, int i, int t) {
        int l = t, r = N;
        while (l + 1 < r) {
            int m = (l + r) / 2;
            cost(j, m) < cost(i - 1, m) ? l = m : r = m;
        }
        return r;
    };

    for (int i = 1, S = -1, L = 0; i < N; i++) {
        while (S >= L) {
            auto [j, t] = deq[S];
            if (cost(i - 1, t) <= cost(j, t)) {
                deq.pop_back(), S--;
            } else {
                break;
            }
        }

        if (int t = S < L ? i : improv(deq[S][0], i, deq[S][1]); t < N) {
            deq.push_back({i - 1, t}), S++;
        }

        E[i] = cost(deq[L][0], i);
        D[i] = dn(E[i], i);

        ++deq[L][1];
        L += L < S && deq[L][1] == deq[L + 1][1];
    }

    return E;
}
