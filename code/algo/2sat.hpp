#pragma once

#include <bits/stdc++.h>
using namespace std;

// Solve 2-SAT in linear time and find SCCs. O(N) ~400ms for N=500K,C=1M
struct twosat_scc {
    int N, C = 0; // N is the number of variables, C number of components
    vector<array<int, 2>> g;
    vector<int> off, assignment, cmap;

    explicit twosat_scc(int N = 0) : N(N) {}

    int add_var() { return N++; }

    void either(int u, int v) {
        u = u < 0 ? 2 * ~u : 2 * u + 1;
        v = v < 0 ? 2 * ~v : 2 * v + 1;
        assert(0 <= u && u < 2 * N && 0 <= v && v < 2 * N);
        g.push_back({u ^ 1, v});
        g.push_back({v ^ 1, u});
    }

    void equal(int u, int v) { either(u, ~v), either(~u, v); }
    void opposite(int u, int v) { either(u, v), either(~u, ~v); }
    void implies(int u, int v) { either(~u, v); }
    void set(int u) { either(u, u); }

    void at_most_one(const vector<int>& vars) {
        int V = vars.size();
        if (V <= 1) {
            return;
        }
        int cur = ~vars[0];
        for (int i = 2; i < V; i++) {
            int next = add_var();
            either(cur, ~vars[i]);
            either(cur, next);
            either(~vars[i], next);
            cur = ~next;
        }
        either(cur, ~vars[1]);
    }

    vector<int> index, lowlink;
    vector<bool> onstack;
    stack<int> S;
    int depth;

    void dfs(int u) {
        index[u] = lowlink[u] = depth++;
        S.push(u), onstack[u] = true;

        for (int e = off[u]; e < off[u + 1]; e++) {
            int v = g[e][1];
            if (!index[v]) {
                dfs(v);
                lowlink[u] = min(lowlink[u], lowlink[v]);
            } else if (onstack[v]) {
                lowlink[u] = min(lowlink[u], index[v]);
            }
        }

        if (index[u] == lowlink[u]) {
            int v, c = C++;
            do {
                v = S.top(), S.pop(), onstack[v] = false;
                cmap[v] = c;
                if (assignment[v >> 1] == -1)
                    assignment[v >> 1] = v & 1;
            } while (u != v);
        }
    }

    bool solve() {
        assignment.assign(N, -1);
        cmap.assign(2 * N, 0);
        index.assign(2 * N, 0);
        lowlink.assign(2 * N, 0);
        onstack.assign(2 * N, false);
        off.assign(2 * N + 1, 0);
        S = stack<int>();
        depth = 1, C = 0;

        sort(begin(g), end(g));

        for (auto [u, v] : g) {
            off[u + 1]++;
        }
        for (int u = 1; u <= 2 * N; u++) {
            off[u] += off[u - 1];
        }
        for (int u = 0; u < 2 * N; u++) {
            if (!index[u]) {
                dfs(u);
            }
        }
        for (int u = 0; u < N; u++) {
            assert(assignment[u] >= 0);
            if (cmap[2 * u] == cmap[2 * u + 1]) {
                return false;
            }
        }
        return true;
    }
};
