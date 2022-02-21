#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Knuth-Morris-Pratt
 * https://en.wikipedia.org/wiki/Knuth-Morris-Pratt_algorithm
 */
class KMP {
    vector<int> table;
    string needle;

  public:
    KMP(string pattern) : needle(move(pattern)) {
        int P = needle.size();
        table.resize(P + 1);
        table[0] = -1;

        int b = 0;
        for (int j = 1; j < P; ++j, ++b) {
            if (needle[j] == needle[b]) {
                table[j] = table[b];
            } else {
                table[j] = b;
                do {
                    b = table[b];
                } while (b >= 0 && needle[j] != needle[b]);
            }
        }
        table[P] = b;
    }

    int lookup(int j) const { return table[j]; }
    int shift(int j) const { return j - table[j]; }
    const string& get_pattern() const { return needle; }
};

int kmp_search(const string& text, const KMP& kmp) {
    const string& needle = kmp.get_pattern();
    int P = needle.size(), T = text.size();
    int i = 0, j = 0;

    while (i <= T - P) {
        while (j < P && text[i + j] == needle[j]) {
            j++;
        }
        if (j == P) {
            return i;
        }
        i += kmp.shift(j);
        j = kmp.lookup(j);
    }

    return -1;
}

vector<int> kmp_search_all(const string& text, const KMP& kmp) {
    const string& needle = kmp.get_pattern();
    int P = needle.size(), T = text.size();
    int i = 0, j = 0;
    vector<int> match;

    while (i <= T - P) {
        while (j < P && text[i + j] == needle[j]) {
            j++;
        }
        if (j == P) {
            match.push_back(i);
        }
        i += kmp.shift(j);
        j = max(0, kmp.lookup(j));
    }

    return match;
}
