#pragma once

#include <bits/stdc++.h>
using namespace std;

uint64_t floor_sum_unsigned(uint64_t n, uint64_t m, uint64_t a, uint64_t b) {
    assert(n < (1LL << 32) && 1 <= m && m < (1LL << 32));
    uint64_t ans = 0;
    while (true) {
        if (a >= m) {
            ans += n * (n - 1) / 2 * (a / m);
            a %= m;
        }
        if (b >= m) {
            ans += n * (b / m);
            b %= m;
        }

        uint64_t y_max = a * n + b;
        if (y_max < m) {
            break;
        }

        n = y_max / m;
        b = y_max % m;
        swap(a, m);
    }
    return ans;
}

//          n-1 | a*i+b |
// Compute  SUM | ----- |  in  O(log²m)
//          i=0 |_  m  _|
// Notice it includes the case i=0 and is not inclusive for n.
int64_t floor_sum(uint64_t n, uint64_t m, int64_t a, int64_t b) {
    assert(n < (1LL << 32) && 1 <= m && m < (1LL << 32));
    uint64_t ans = 0;
    if (a < 0) {
        int64_t a2 = a % m;
        a2 = a2 < 0 ? a2 + m : a;
        ans -= 1ULL * n * (n - 1) / 2 * ((a2 - a) / m);
        a = a2;
    }
    if (b < 0) {
        int64_t b2 = b % m;
        b2 = b2 < 0 ? b2 + m : b;
        ans -= 1ULL * n * ((b2 - b) / m);
        b = b2;
    }
    return ans + floor_sum_unsigned(n, m, a, b);
}
