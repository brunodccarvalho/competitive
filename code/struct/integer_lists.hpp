#pragma once

#include <bits/stdc++.h>
using namespace std;

struct linked_lists {
    int L, N;
    vector<int> next, prev;

    // L: lists are [0...L), N: integers are [0...N)
    explicit linked_lists(int L = 0, int N = 0) { assign(L, N); }

    int rep(int l) const { return l + N; }
    int head(int l) const { return next[rep(l)]; }
    int tail(int l) const { return prev[rep(l)]; }
    bool empty(int l) const { return next[rep(l)] == rep(l); }

    int add_list() { return next.push_back(rep(L)), prev.push_back(rep(L)), L++; }
    void clear(int l) { assert(0 <= l && l < L), next[rep(l)] = prev[rep(l)] = rep(l); }

    void init(int l, int n) { meet(rep(l), n, rep(l)); }
    void push_front(int l, int n) { meet(rep(l), n, head(l)); }
    void push_back(int l, int n) { meet(tail(l), n, rep(l)); }
    void insert_before(int i, int n) { meet(prev[i], n, i); }
    void insert_after(int i, int n) { meet(i, n, next[i]); }
    void erase(int n) { meet(prev[n], next[n]); }
    void pop_front(int l) { meet(rep(l), next[head(l)]); }
    void pop_back(int l) { meet(prev[tail(l)], rep(l)); }
    void cut_suffix(int l, int x) { meet(prev[x], rep(l)); }
    void cut_prefix(int l, int x) { meet(rep(l), next[x]); }

    void splice_suffix_back(int l, int b, int x) { // move b[x,...) to back of l
        assert(0 <= l && l < L && 0 <= b && b < L && l != b && !empty(b));
        int y = tail(b);
        meet(prev[x], rep(b)), meet(tail(l), x), meet(y, rep(l));
    }
    void splice_suffix_front(int l, int b, int x) { // move b[x,...) to front of l
        assert(0 <= l && l < L && 0 <= b && b < L && l != b && !empty(b));
        int y = tail(b);
        meet(prev[x], rep(b)), meet(y, head(l)), meet(rep(l), x);
    }
    void splice_prefix_back(int l, int b, int y) { // move b(...,y] to back of l
        assert(0 <= l && l < L && 0 <= b && b < L && l != b && !empty(b));
        int x = head(b);
        meet(rep(b), next[y]), meet(tail(l), x), meet(y, rep(l));
    }
    void splice_prefix_front(int l, int b, int y) { // move b(...,y] to front of l
        assert(0 <= l && l < L && 0 <= b && b < L && l != b && !empty(b));
        int x = head(b);
        meet(rep(b), next[y]), meet(y, head(l)), meet(rep(l), x);
    }
    void splice_front(int l, int b) {
        assert(0 <= l && l < L && 0 <= b && b < L && l != b && !empty(b));
        meet(tail(b), head(l)), meet(rep(l), head(b)), clear(b);
    }
    void splice_back(int l, int b) {
        assert(0 <= l && l < L && 0 <= b && b < L && l != b && !empty(b));
        meet(tail(l), head(b)), meet(tail(b), rep(l)), clear(b);
    }

    void clear() {
        iota(begin(next) + N, end(next), N);
        iota(begin(prev) + N, end(prev), N);
    }
    void assign(int L, int N) {
        this->L = L, this->N = N;
        next.resize(N + L), prev.resize(N + L), clear();
    }

  private:
    inline void meet(int u, int v) { next[u] = v, prev[v] = u; }
    inline void meet(int u, int v, int w) { meet(u, v), meet(v, w); }
};

#define FOR_EACH_IN_LINKED_LIST(i, l, lists) \
    for (int z##i = l, i = lists.head(z##i); i != lists.rep(z##i); i = lists.next[i])

#define FOR_EACH_IN_LINKED_LIST_REVERSE(i, l, lists) \
    for (int z##i = l, i = lists.tail(z##i); i != lists.rep(z##i); i = lists.prev[i])

struct forward_lists {
    int L, N;
    vector<int> next;

    // L: lists are [0...L), N: integers are [0...N)
    explicit forward_lists(int L = 0, int N = 0) { assign(L, N); }

    int rep(int l) const { return l + N; }
    int head(int l) const { return next[rep(l)]; }
    bool empty(int l) const { return head(l) == -1; }

    void init(int l, int n) { next[rep(l)] = n, next[n] = -1; }
    void clear(int l) { next[rep(l)] = -1; }
    void exchange(int l, int t) { swap(next[rep(l)], next[rep(t)]); }

    void push(int l, int n) { insert(rep(l), n); }
    void insert(int i, int n) { next[n] = next[i], next[i] = n; }
    void pop(int l) { assert(head(l) != -1), next[rep(l)] = next[head(l)]; }

    void clear() { fill(begin(next) + N, end(next), -1); }
    void assign(int L, int N) { this->L = L, this->N = N, next.assign(L + N, -1); }
};

#define FOR_EACH_IN_FORWARD_LIST(i, l, lists) \
    for (int i = lists.head(l); i != -1; i = lists.next[i])
