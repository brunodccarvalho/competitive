#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Boyer Moore
 * https://en.wikipedia.org/wiki/Boyer-Moore_string-search_algorithm
 */
class BoyerMoore {
    vector<short> finger;
    vector<vector<int>> bad;
    vector<int> good;
    string needle;

  public:
    BoyerMoore(string pattern) : needle(move(pattern)) {
        int P = needle.size();
        finger.assign(256, -1);
        good.assign(P + 1, 0);

        // bad character rule
        for (int i = 0; i < P; i++) {
            int c = needle[i];
            if (finger[c] == -1) {
                finger[c] = bad.size();
                bad.emplace_back(P, 0);
            }
            for (int j = i + 1; j < P; j++) {
                bad[finger[c]][j] = i;
            }
        }

        // good suffix rule
        vector<int> border(P + 1, 0);
        int b = P + 1;
        border[P] = b;
        for (int i = P; i > 0; i--) {
            while (b <= P && needle[i - 1] != needle[b - 1]) {
                if (good[b] == 0) {
                    good[b] = b - i;
                }
                b = border[b];
            }
            border[i - 1] = --b;
        }

        for (int i = 0; i <= P; i++) {
            if (good[i] == 0) {
                good[i] = b;
            }
            if (i == b) {
                b = border[b];
            }
        }
    }

    int mismatch_shift(int j, char c) const {
        return max(bad_shift(j, c), good_shift(j));
    }
    int match_shift() const { return good[0]; }
    int bad_shift(int j, char c) const {
        return finger[c] == -1 ? j + 1 : j - bad[finger[c]][j];
    }
    int good_shift(int j) const { return good[j + 1]; }
    const string& get_pattern() const { return needle; }
};

int boyer_moore_search(const string& text, const BoyerMoore& bm) {
    const string& needle = bm.get_pattern();
    int P = needle.size(), T = text.size();
    int i = 0, j = P - 1;

    while (i <= T - P) {
        while ((j >= 0) && (text[i + j] == needle[j])) {
            --j;
        }
        if (j < 0) {
            return i;
        }
        i += bm.mismatch_shift(j, text[i + j]);
        j = P - 1;
    }

    return -1;
}

vector<int> boyer_moore_search_all(const string& text, const BoyerMoore& bm) {
    const string& needle = bm.get_pattern();
    int P = needle.size(), T = text.size();
    int i = 0, j = P - 1, g = 0;
    vector<int> match;

    while (i <= T - P) {
        while (j >= g && text[i + j] == needle[j]) {
            --j;
        }
        if (j < g) {
            match.push_back(i);
            i += bm.match_shift();
            g = P - bm.match_shift();
        } else {
            i += bm.mismatch_shift(j, text[i + j]);
            g = 0;
        }
        j = P - 1;
    }

    return match;
}
