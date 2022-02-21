#pragma once

#include "random.hpp"

// Generate all strings, with length [minlen,maxlen] with characters [a,b]
auto generate_all_strings(int minlen, int maxlen, char a = 'a', char b = 'z') {
    assert(pow(b - a + 1, maxlen) < 1e8);
    vector<string> strs{""};
    int L = 0, S = 1, k = 1, splice_index = 0;
    for (int len = 1; len <= maxlen; len++) {
        if (len == minlen)
            splice_index = S;
        strs.resize(S + (b - a + 1) * (S - L));
        for (char c = a; c <= b; c++) {
            for (int i = L; i < S; i++) {
                strs[k++] = strs[i] + c;
            }
        }
        L = S, S = k;
    }
    strs.erase(begin(strs), begin(strs) + splice_index);
    return strs;
}
