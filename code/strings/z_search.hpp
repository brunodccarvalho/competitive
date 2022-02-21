#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Z algorithm (prefix length array)
 * https://www.hackerearth.com/practice/algorithms/string-algorithm/z-algorithm/tutorial
 */
int z_search(const string& text, const string& needle, char null = '\0') {
    string s = needle + null + text;
    int P = needle.size(), S = s.size();
    vector<int> z(S);
    int L = 0, R = 0;
    for (int i = 1; i < S; i++) {
        if (i < R && z[i - L] < R - i) {
            z[i] = z[i - L];
        } else {
            L = i;
            R = max(R, i);
            while (R < S && s[R - L] == s[R]) {
                R++;
            }
            z[i] = R - L;
        }
        if (i > P && z[i] == P) {
            return i - (P + 1);
        }
    }
    return -1;
}

vector<int> z_search_all(const string& text, const string& needle, char null = '\0') {
    string s = needle + null + text;
    int P = needle.size(), S = s.size();
    vector<int> z(S);
    int L = 0, R = 0;
    vector<int> match;

    for (int i = 1; i < S; i++) {
        if (i < R && z[i - L] < R - i) {
            z[i] = z[i - L];
        } else {
            L = i;
            R = max(R, i);
            while (R < S && s[R - L] == s[R]) {
                R++;
            }
            z[i] = R - L;
        }
        if (i > P && z[i] == P) {
            match.push_back(i - (P + 1));
        }
    }
    return match;
}
