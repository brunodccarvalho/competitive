#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Levenshtein (edit) distance from a to b
 * del: cost of deletion of one character (from a)
 * ins: cost of insertion of one character (from b into a)
 * sub: cost of substitution of one character
 * O(ab) time, O(b) memory without recovery
 */
int levenshtein_distance(const string& a, const string& b, int del, int ins, int sub) {
    int A = a.size(), B = b.size();
    vector<int> prev(B + 1, 0);
    vector<int> next(B + 1, 0);

    for (int j = 0; j < B; j++)
        prev[j + 1] = (j + 1) * ins;

    for (int i = 0; i < A; i++) {
        next[0] = (i + 1) * del;
        for (int j = 0; j < B; j++) {
            int eqs = a[i] == b[j] ? 0 : sub;
            int del_dist = del + next[j];
            int ins_dist = ins + prev[j + 1];
            int sub_dist = eqs + prev[j];
            next[j + 1] = min(min(ins_dist, del_dist), sub_dist);
        }
        swap(prev, next);
    }

    return prev[B];
}

/**
 * Restricted damerau distance from a to b
 * del: cost of deletion of one character (from a)
 * ins: cost of insertion of one character (from b into a)
 * sub: cost of substitution of one character
 * tra: cost of transposition of two characters (in a)
 * O(ab) time, O(b) memory without recovery
 */
int simple_damerau_distance(const string& a, const string& b, int del, int ins, int sub,
                            int tra) {
    int A = a.size(), B = b.size();
    vector<int> tran(B + 1, 0);
    vector<int> prev(B + 1, 0);
    vector<int> next(B + 1, 0);

    for (int j = 0; j < B; j++)
        prev[j + 1] = (j + 1) * ins;

    for (int i = 0; i < A; i++) {
        next[0] = (i + 1) * del;
        for (int j = 0; j < B; j++) {
            int eqs = a[i] == b[j] ? 0 : sub;
            int del_dist = del + next[j];
            int ins_dist = ins + prev[j + 1];
            int sub_dist = eqs + prev[j];
            next[j + 1] = min(min(ins_dist, del_dist), sub_dist);
            if (i && j && a[i] == b[j - 1] && a[i - 1] == b[j]) {
                int tra_dist = tra + tran[j - 1];
                next[j + 1] = min(next[j + 1], tra_dist);
            }
        }

        swap(tran, next);
        swap(prev, tran);
    }

    return prev[B];
}

/**
 * Damerau distance from a to b
 * del: cost of deletion of one character (from a)
 * ins: cost of insertion of one character (from b into a)
 * sub: cost of substitution of one character
 * tra: cost of transposition of two characters (in a)
 * O(ab) time, O(ab) memory
 */
int damerau_distance(const string& a, const string& b, int del, int ins, int sub,
                     int tra) {
    int A = a.size(), B = b.size();
    vector<vector<int>> dp(A + 2, vector<int>(B + 2, 0));
    array<int, 256> finger = {};

    const int inf = (A + B) * max(max(del, ins), max(sub, tra));
    fill(begin(dp[0]), end(dp[0]), inf);

    dp[1][0] = inf;
    for (int j = 0; j < B; j++) {
        dp[1][j + 2] = (j + 1) * ins;
    }

    for (int i = 0; i < A; i++) {
        dp[i + 2][0] = inf;
        dp[i + 2][1] = (i + 1) * del;
        int jp = 0; // the largest jj<j for which a[i] == b[jj]
        for (int j = 0; j < B; j++) {
            int eqs = a[i] == b[j] ? 0 : sub;
            int ip = finger[int(b[j])]; // the largest ii<i for which a[ii] == b[j]
            int swp = (i - ip + j - jp + 1) * tra;
            int del_dist = del + dp[i + 2][j + 1];
            int ins_dist = ins + dp[i + 1][j + 2];
            int sub_dist = eqs + dp[i + 1][j + 1];
            int tra_dist = swp + dp[ip][jp];
            dp[i + 2][j + 2] = min(min(ins_dist, del_dist), min(sub_dist, tra_dist));
            if (a[i] == b[j])
                jp = j + 1;
        }
        finger[int(a[i])] = i + 1;
    }

    return dp[A + 1][B + 1];
}
