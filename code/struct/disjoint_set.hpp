#pragma once

#include <bits/stdc++.h>
using namespace std;

struct disjoint_set {
    int N, S;
    vector<int> next, size;

    explicit disjoint_set(int N = 0) : N(N), S(N), next(N), size(N, 1) {
        iota(begin(next), end(next), 0);
    }

    void assign(int n) { *this = disjoint_set(n); }
    void reset() { *this = disjoint_set(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    bool unit(int i) { return i == next[i] && size[i] == 1; }
    bool root(int i) { return find(i) == i; }
    void reroot(int u) {
        if (int r = find(u); u != r) {
            size[u] = size[r];
            next[u] = next[r] = u;
        }
    }

    int find(int i) {
        while (i != next[i]) {
            i = next[i] = next[next[i]];
        }
        return i;
    }

    bool join(int i, int j) {
        i = find(i);
        j = find(j);
        if (i != j) {
            if (size[i] < size[j]) {
                swap(i, j);
            }
            next[j] = i;
            size[i] += size[j];
            S--;
            return true;
        }
        return false;
    }
};

struct disjoint_set_rollback {
    int N, S;
    vector<int> next;
    vector<pair<int, int>> history;

    explicit disjoint_set_rollback(int N = 0) : N(N), S(N), next(N, -1) {}

    void assign(int n) { *this = disjoint_set_rollback(n); }
    void reset() { *this = disjoint_set_rollback(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    bool unit(int i) { return next[i] == -1; }
    bool root(int i) { return next[i] < 0; }
    int size(int i) { return -next[find(i)]; }
    int time() const { return history.size(); }

    void rollback(int t) {
        int i = time();
        while (i > t) {
            i--, next[history[i].first] = history[i].second;
            i--, next[history[i].first] = history[i].second;
            S++;
        }
        history.resize(t);
    }

    int find(int i) {
        while (next[i] >= 0) {
            i = next[i];
        }
        return i;
    }

    bool join(int i, int j) {
        i = find(i);
        j = find(j);
        if (i != j) {
            if (size(i) < size(j)) {
                swap(i, j);
            }
            history.emplace_back(i, next[i]);
            history.emplace_back(j, next[j]);
            next[i] += next[j];
            next[j] = i, S--;
            return true;
        }
        return false;
    }
};

struct sparse_disjoint_set {
    int N, S;
    unordered_map<int, int> entry;

    explicit sparse_disjoint_set(int N = 0) : N(N), S(N) {}

    int count(int i) {
        auto pos = entry.find(i);
        return pos == entry.end() ? 1 : -pos->second;
    }
    void assign(int n) { *this = sparse_disjoint_set(n); }
    void reset() { *this = sparse_disjoint_set(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    bool unit(int i) { return count(i) == 1; }
    bool root(int i) { return find(i) == i; }
    void reroot(int i) {
        int j = find(i);
        if (j != i) {
            entry[i] = entry[j];
            entry[j] = i;
        }
    }
    int find(int i) {
        auto pos = entry.find(i);
        while (pos != entry.end() && pos->second >= 0) {
            auto nxt = entry.find(pos->second);
            pos->second = nxt->second >= 0 ? nxt->second : pos->second;
            i = nxt->first, pos = nxt;
        }
        return i;
    }
    bool join(int i, int j) {
        i = find(i);
        j = find(j);
        if (i != j) {
            auto pi = entry.emplace(i, -1).first;
            auto pj = entry.emplace(j, -1).first;
            if (-pi->second < -pj->second) {
                swap(i, j), swap(pi, pj);
            }
            pi->second += pj->second;
            pj->second = i;
            S--;
            return true;
        }
        return false;
    }
};
