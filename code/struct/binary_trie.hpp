#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Maintain a (non-compressed, counted) trie of integers with B bits
 * Answer the following queries: min xor, max xor, and range xor count
 * All queries and updates are O(B)
 * Memory: 3NB integers
 */
template <typename T, int B>
struct binary_trie {
    using Count = int;
    vector<array<int, 2>> kids;
    vector<Count> count;

    binary_trie() : kids(1, {0, 0}), count(1, 0) {}

    Count insert(T x, Count add = 1, Count maximum = 1) {
        int u = 0;
        for (int b = B - 1; b >= 0; b--) {
            u = access(u, x >> b & 1);
        }
        if (count[u] >= maximum) {
            return 0;
        }
        add = min(add, maximum - count[u]);
        u = 0;
        for (int b = B - 1; b >= 0; b--) {
            u = kids[u][x >> b & 1];
            count[u] += add;
        }
        return add;
    }

    Count erase(T x, Count rem = 1, Count minimum = 0) {
        int u = 0;
        for (int b = B - 1; b >= 0; b--) {
            u = kids[u][x >> b & 1];
            if (count[u] <= minimum) {
                return 0;
            }
        }
        rem = min(rem, count[u] - minimum);
        u = 0;
        for (int b = B - 1; b >= 0; b--) {
            u = kids[u][x >> b & 1];
            count[u] -= rem;
        }
        return rem;
    }

    Count contains(T x) const {
        int u = 0;
        for (int b = B - 1; b >= 0; b--) {
            u = kids[u][x >> b & 1];
            if (u == 0) {
                return 0;
            }
        }
        return count[u];
    }

    // Compute argmin{y in S}(x ^ y)
    auto argxormin(T x) const {
        T y = 0;
        for (int u = 0, b = B - 1; b >= 0; b--) {
            int bit = x >> b & 1;
            int v = kids[u][bit], w = kids[u][!bit];
            if (count[v] > 0) {
                y |= T(bit) << b;
                u = v;
            } else {
                y |= T(!bit) << b;
                u = w;
            }
        }
        return y;
    }

    // Compute argmax{y in S}(x ^ y)
    auto argxormax(T x) const {
        T y = 0;
        for (int u = 0, b = B - 1; b >= 0; b--) {
            int bit = x >> b & 1;
            int v = kids[u][!bit], w = kids[u][bit];
            if (count[v] > 0) {
                y |= T(!bit) << b;
                u = v;
            } else {
                y |= T(bit) << b;
                u = w;
            }
        }
        return y;
    }

    // Compute |{y in S : x ^ y < r}|
    Count count_xor_prefix(T x, T r) const {
        if (r-- <= 0) {
            return 0;
        }
        Count ans = 0;
        int b = B - 1, u = 0;
        do {
            int xb = x >> b & 1, rb = r >> b & 1;
            ans += rb ? count[kids[u][xb]] : 0;
            u = kids[u][xb ^ rb];
        } while (--b >= 0 && u);
        ans += b == -1 ? count[u] : 0;
        return ans;
    }

    // Compute |{y in S : l <= x ^ y < r}|
    Count count_xor_range(T x, T l, T r) const {
        return l < r ? count_xor_prefix(x, r) - count_xor_prefix(x, l) : 0;
    }

  private:
    int access(int u, int i) {
        if (kids[u][i] == 0) {
            kids[u][i] = kids.size();
            kids.push_back({0, 0});
            count.push_back(0);
        }
        return kids[u][i];
    }
};
