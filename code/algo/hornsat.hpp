#pragma once

#include <bits/stdc++.h>
using namespace std;

// Solve horn-sat and compute assignment. O(n)
// Usage:
//   vector<vector<int>> clauses;
//   ... first variable in clause is positive, rest is negated ...
//   vector<bool> assignment;
//   bool satisfiable = hornsat(clauses, &assignment);
bool hornsat(vector<vector<int>> clauses, vector<bool>* assignment = nullptr) {
    int N = 0, C = clauses.size();
    for (int c = 0; c < C; c++) {
        int S = clauses[c].size();
        assert(S >= 1);
        for (int i = 0; i < S; i++) {
            int var = clauses[c][i];
            N = max(N, var + 1);
            assert((i == 0 && var == -1) || var >= 0);
        }
        if (S == 1 && clauses[c][0] == -1) {
            return false;
        }
    }

    vector<int> units;
    vector<int> unvisited(C);
    vector<vector<int>> in_tail(N);
    vector<bool> solution(N);

    for (int c = 0; c < C; c++) {
        int S = clauses[c].size();
        if (S == 1) {
            units.push_back(c);
        }
        unvisited[c] = S - 1;
        for (int i = 1; i < S; i++) {
            in_tail[clauses[c][i]].push_back(i);
        }
    }

    while (!units.empty()) {
        int c = units.back();
        units.pop_back();
        int head = clauses[c][0];
        if (head == -1) {
            return false;
        }
        if (solution[head]) {
            continue;
        }
        solution[head] = true;
        for (int j : in_tail[head]) {
            if (--unvisited[j] == 0) {
                units.push_back(j);
            }
        }
    }

    if (assignment) {
        *assignment = move(solution);
    }
    return true;
}
