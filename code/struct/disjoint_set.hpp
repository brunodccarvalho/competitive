#pragma once

#include <bits/stdc++.h>
using namespace std;

struct disjoint_set {
    int N, S;
    vector<int> nxt, siz;

    explicit disjoint_set(int N = 0) : N(N), S(N), nxt(N), siz(N, 1) {
        iota(begin(nxt), end(nxt), 0);
    }

    void assign(int n) { *this = disjoint_set(n); }
    void reset() { *this = disjoint_set(N); }
    int size(int n) { return siz[n]; }
    bool same(int i, int j) { return find(i) == find(j); }
    bool isroot(int i) { return find(i) == i; }
    void reroot(int u) {
        if (int r = find(u); u != r) {
            siz[u] = siz[r];
            nxt[u] = nxt[r] = u;
        }
    }

    int find(int i) {
        while (i != nxt[i]) {
            i = nxt[i] = nxt[nxt[i]];
        }
        return i;
    }

    bool join(int i, int j) {
        i = find(i);
        j = find(j);
        if (i != j) {
            if (siz[i] < siz[j]) {
                swap(i, j);
            }
            nxt[j] = i;
            siz[i] += siz[j];
            S--;
            return true;
        }
        return false;
    }
};

struct disjoint_set_rollback {
    int N, S;
    vector<int> nxt;
    vector<pair<int, int>> hist;

    explicit disjoint_set_rollback(int N = 0) : N(N), S(N), nxt(N, -1) {}

    void assign(int n) { *this = disjoint_set_rollback(n); }
    void reset() { *this = disjoint_set_rollback(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    bool unit(int i) { return nxt[i] == -1; }
    bool root(int i) { return nxt[i] < 0; }
    int size(int i) { return -nxt[find(i)]; }
    int time() const { return hist.size(); }

    void rollback(int t) {
        int i = time();
        while (i > t) {
            i--, nxt[hist[i].first] = hist[i].second;
            i--, nxt[hist[i].first] = hist[i].second;
            S++;
        }
        hist.resize(t);
    }

    int find(int i) {
        while (nxt[i] >= 0) {
            i = nxt[i];
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
            hist.emplace_back(i, nxt[i]);
            hist.emplace_back(j, nxt[j]);
            nxt[i] += nxt[j];
            nxt[j] = i, S--;
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
