#pragma once

#include "integer_lists.hpp"

struct disjoint_set {
    int N, S;
    vector<int> nxt, siz;

    explicit disjoint_set(int N = 0) : N(N), S(N), nxt(N), siz(N, 1) {
        iota(begin(nxt), end(nxt), 0);
    }

    void assign(int n) { *this = disjoint_set(n); }
    void reset() { *this = disjoint_set(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    int size(int i) { return siz[find(i)]; }

    void reroot(int u) {
        if (int r = find(u); u != r) {
            siz[u] = siz[r];
            nxt[u] = nxt[r] = u;
        }
    }

    int find(int i) { return i != nxt[i] ? nxt[i] = find(nxt[i]) : i; }

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
    vector<int> nxt, siz;
    vector<pair<int, int>> hist;

    explicit disjoint_set_rollback(int N = 0) : N(N), S(N), nxt(N), siz(N, 1) {
        iota(begin(nxt), end(nxt), 0);
    }

    void assign(int n) { *this = disjoint_set_rollback(n); }
    void reset() { *this = disjoint_set_rollback(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    int size(int i) { return siz[find(i)]; }
    int time() const { return hist.size(); }

    void rollback(int t) {
        int i = time();
        assert(i >= t);
        while (i > t) {
            auto [a, b] = hist[--i];
            siz[a] -= siz[b], nxt[b] = b, S++;
        }
        hist.resize(t);
    }

    int find(int i) {
        while (i != nxt[i]) {
            i = nxt[i];
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
            hist.push_back({i, j});
            siz[i] += siz[j];
            nxt[j] = i;
            S--;
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

struct bipartite_disjoint_set {
    static constexpr int DEAD = -1;
    int N, S, B = 0;
    vector<int> nxt, siz, col;

    explicit bipartite_disjoint_set(int N = 0) : N(N), S(N), nxt(N), siz(N, 1), col(N) {
        iota(begin(nxt), end(nxt), 0);
    }

    void assign(int n) { *this = bipartite_disjoint_set(n); }
    void reset() { *this = bipartite_disjoint_set(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    bool bipartite() const { return B == 0; }
    bool bipartite(int i) { return color(i) != DEAD; }
    int size(int i) { return siz[find(i)]; }
    int color(int i) { return find(i), col[i]; }

    int find(int i) {
        for (int j = nxt[i]; j != nxt[j]; j = nxt[j]) {
            update(col[i], col[j]);
            nxt[i] = nxt[j];
        }
        update(col[i], col[nxt[i]]);
        return nxt[i];
    }

    bool join(int i, int j) {
        assert(i != j); // ambiguous handling
        int a = find(i);
        int b = find(j);
        if (a == b && col[i] == col[j]) {
            col[a] = DEAD, B++;
        } else if (a != b) {
            if (siz[a] < siz[b]) {
                swap(a, b);
            }
            nxt[b] = a;
            siz[a] += siz[b], S--;
            col[b] ^= col[i] != DEAD && col[j] != DEAD && col[i] == col[j];
        }
        return a != b;
    }

  private:
    void update(int& a, int b) { a = a != DEAD && b != DEAD ? a ^ b : DEAD; }
};

struct bipartite_disjoint_set_rollback {
    static constexpr int DEAD = -1;
    int N, S, B = 0;
    vector<int> nxt, siz, col;
    vector<array<int, 3>> hist;

    explicit bipartite_disjoint_set_rollback(int N = 0)
        : N(N), S(N), nxt(N), siz(N, 1), col(N) {
        iota(begin(nxt), end(nxt), 0);
    }

    void assign(int n) { *this = bipartite_disjoint_set_rollback(n); }
    void reset() { *this = bipartite_disjoint_set_rollback(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    bool bipartite() const { return B == 0; }
    bool bipartite(int i) { return color(i) != DEAD; }
    int size(int i) { return siz[find(i)]; }
    int time() const { return hist.size(); }

    void rollback(int t) {
        int i = time();
        assert(i >= t);
        while (i > t) {
            auto [a, b, c] = hist[--i];
            if (b == N) {
                col[a] = c, B--;
            } else {
                siz[a] -= siz[b], nxt[b] = b, col[b] = c, S++;
            }
        }
        hist.resize(t);
    }

    int color(int i) {
        int c = col[i];
        while (i != nxt[i]) {
            i = nxt[i], update(c, col[i]);
        }
        return c;
    }

    int find(int i) {
        while (i != nxt[i]) {
            i = nxt[i];
        }
        return i;
    }

    bool join(int i, int j) {
        assert(i != j); // ambiguous handling
        int a = find(i);
        int b = find(j);
        int ci = color(i);
        int cj = color(j);
        if (a == b && ci == cj) {
            hist.push_back({a, N, col[a]});
            col[a] = DEAD, B++;
        } else if (a != b) {
            if (siz[a] < siz[b]) {
                swap(a, b);
            }
            hist.push_back({a, b, col[b]});
            siz[a] += siz[b];
            nxt[b] = a;
            nxt[b] = a;
            S--;
            col[b] ^= ci != DEAD && cj != DEAD && ci == cj;
        }
        return a != b;
    }

  private:
    void update(int& a, int b) { a = a != DEAD && b != DEAD ? a ^ b : DEAD; }
};

struct tracking_disjoint_set {
    int N, S;
    vector<int> nxt, siz;
    linked_lists ll;

    explicit tracking_disjoint_set(int N = 0) : N(N), S(N), nxt(N), siz(N, 1), ll(N, N) {
        iota(begin(nxt), end(nxt), 0);
        for (int u = 0; u < N; u++) {
            ll.init(u, u);
        }
    }

    void assign(int n) { *this = tracking_disjoint_set(n); }
    void reset() { *this = tracking_disjoint_set(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    int size(int i) { return siz[find(i)]; }

    void reroot(int u) {
        if (int r = find(u); u != r) {
            siz[u] = siz[r];
            nxt[u] = nxt[r] = u;
            ll.splice_back(u, r);
        }
    }

    int find(int i) { return i != nxt[i] ? nxt[i] = find(nxt[i]) : i; }

    bool join(int i, int j) {
        i = find(i);
        j = find(j);
        if (i != j) {
            if (siz[i] < siz[j]) {
                swap(i, j);
            }
            siz[i] += siz[j];
            nxt[j] = i;
            S--;
            ll.splice_back(i, j);
            return true;
        }
        return false;
    }
};

struct tracking_disjoint_set_rollback {
    int N, S;
    vector<int> nxt, siz;
    vector<pair<int, int>> hist;
    linked_lists ll;

    explicit tracking_disjoint_set_rollback(int N = 0)
        : N(N), S(N), nxt(N), siz(N, 1), ll(N, N) {
        iota(begin(nxt), end(nxt), 0);
        for (int u = 0; u < N; u++) {
            ll.init(u, u);
        }
    }

    void assign(int n) { *this = tracking_disjoint_set_rollback(n); }
    void reset() { *this = tracking_disjoint_set_rollback(N); }
    bool same(int i, int j) { return find(i) == find(j); }
    int size(int i) { return siz[find(i)]; }
    int time() const { return hist.size(); }

    void rollback(int t) {
        int i = time();
        assert(i >= t);
        while (i > t) {
            auto [a, b] = hist[--i];
            siz[a] -= siz[b], nxt[b] = b, S++;
            ll.splice_suffix_back(b, a, b);
        }
        hist.resize(t);
    }

    int find(int i) {
        while (i != nxt[i]) {
            i = nxt[i];
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
            hist.push_back({i, j});
            siz[i] += siz[j];
            nxt[j] = i;
            S--;
            ll.splice_back(i, j);
            return true;
        }
        return false;
    }
};
