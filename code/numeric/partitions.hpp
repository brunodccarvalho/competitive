#pragma once

#include "hash.hpp"
#include "random.hpp"

vector<int> first_choice(int n, int k, int s) {
    vector<int> comb(k);
    iota(begin(comb), end(comb), 1), (void)n;
    return comb;
}

bool next_choice(vector<int>& comb, int n, int s) {
    for (int k = comb.size(), i = k - 1; i >= 0; i--) {
        if (comb[i] <= n - k + i + s) {
            comb[i]++;
            for (int j = i + 1; j < k; j++) {
                comb[j] = comb[j - 1] + 1;
            }
            return true;
        }
    }
    return false;
}

vector<int> first_unsized_partition(int n) { return vector<int>(n, 1); }

bool next_unsized_partition(vector<int>& a, bool self = true) {
    if (self && int(a.size()) == 1) {
        a.assign(a[0], 1);
        return false;
    }
    int k = a.size() - 1;
    int x = a[k - 1] + 1;
    int y = a[k] - 1;
    k -= 1;
    while (x <= y) {
        a.resize(k + 1);
        a[k] = x;
        y -= x;
        k += 1;
    }
    a.resize(k + 1);
    a[k] = x + y;
    return self || k > 0;
}
