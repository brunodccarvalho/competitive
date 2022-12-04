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
    void wrap(int n) { meet(n, n); }

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

// A fast vector-based hashset (chinese doubly linked list)
struct fast_set {
    int N = 0, S = 0;
    vector<int> next, prev;

    fast_set() = default;
    fast_set(int N) : N(N), next(N + 1, -1), prev(N + 1, -1) { prev[N] = next[N] = N; }

    int head() const { return next[N]; }
    int tail() const { return prev[N]; }
    int front() const { return assert(next[N] != N), next[N]; }
    int back() const { return assert(prev[N] != N), prev[N]; }

    int add_front(int n) { //
        return next[n] == -1 ? meet(N, n, next[N]), S++, 1 : 0;
    }
    int add_back(int n) { //
        return next[n] == -1 ? meet(prev[N], n, N), S++, 1 : 0;
    }
    int del(int n) { //
        return next[n] != -1 ? meet(prev[n], next[n]), clr(n), S--, 1 : 0;
    }

    void add_before(int n, int pos) {
        assert(next[n] == -1 && next[pos] != -1);
        meet(prev[pos], n, pos), S++;
    }
    void add_after(int n, int pos) {
        assert(next[n] == -1 && next[pos] != -1);
        meet(pos, n, next[pos]), S++;
    }
    int del_after(int n) {
        assert(next[n] != -1);
        int k = next[n];
        return meet(prev[n], next[n]), clr(n), S--, k;
    }

    int pop_front() {
        int u = front();
        return del(u), u;
    }
    int pop_back() {
        int u = back();
        return del(u), u;
    }

    bool has(int n) const { return next[n] != -1; }
    int size() const { return S; }
    int universe() const { return N; }
    bool empty() const { return S == 0; }

    void clear() {
        while (next[N] != N) {
            int u = next[N];
            meet(N, next[u]);
            clr(u), S--;
        }
    }

    auto to_vec() const {
        vector<int> us;
        for (int u = next[N]; u != N; u = next[u]) {
            us.push_back(u);
        }
        return us;
    }

  private:
    void meet(int u, int v) { next[u] = v, prev[v] = u; }
    void meet(int u, int v, int w) { meet(u, v), meet(v, w); }
    void clr(int u) { prev[u] = next[u] = -1; }
};

#define LOOP_FAST_SET(u, set) for (int u = set.head(); u != set.N; u = set.next[u])

#define LOOP_FAST_SET_DEL(u, set) \
    for (int u = set.head(), ahead##u = set.next[u]; u != set.N; u = ahead##u, ahead##u = set.next[u])

// A fast vector-based hashset (chinese doubly linked list) with global hash tracking
struct fast_hashing_set {
    int N = 0, S = 0;
    uint64_t H = 0;
    vector<int> next, prev;

    fast_hashing_set() = default;
    fast_hashing_set(int N) : N(N), next(N + 1, -1), prev(N + 1, -1) {
        prev[N] = next[N] = N;
    }

    int head() const { return next[N]; }
    int tail() const { return prev[N]; }
    int front() const { return assert(next[N] != N), next[N]; }
    int back() const { return assert(prev[N] != N), prev[N]; }

    int add_front(int n) { //
        return next[n] == -1 ? meet(N, n, next[N]), S++, H ^= nhash[n], 1 : 0;
    }
    int add_back(int n) { //
        return next[n] == -1 ? meet(prev[N], n, N), S++, H ^= nhash[n], 1 : 0;
    }
    int del(int n) { //
        return next[n] != -1 ? meet(prev[n], next[n]), clr(n), S--, H ^= nhash[n], 1 : 0;
    }

    void add_before(int n, int pos) {
        assert(next[n] == -1 && next[pos] != -1);
        meet(prev[pos], n, pos), S++, H ^= nhash[n];
    }
    void add_after(int n, int pos) {
        assert(next[n] == -1 && next[pos] != -1);
        meet(pos, n, next[pos]), S++, H ^= nhash[n];
    }
    int del_after(int n) {
        assert(next[n] != -1);
        int k = next[n];
        return meet(prev[n], next[n]), clr(n), S--, H ^= nhash[n], k;
    }

    int pop_front() {
        int u = front();
        return del(u), u;
    }
    int pop_back() {
        int u = back();
        return del(u), u;
    }

    bool has(int n) const { return next[n] != -1; }
    int size() const { return S; }
    int universe() const { return N; }
    bool empty() const { return S == 0; }
    uint64_t hash() const { return H; }

    void clear() {
        while (next[N] != N) {
            int u = next[N];
            meet(N, next[u]);
            clr(u), H ^= nhash[u], S--;
        }
    }

    auto to_vec() const {
        vector<int> us;
        for (int u = next[N]; u != N; u = next[u]) {
            us.push_back(u);
        }
        return us;
    }

    static void prepare_hashes(int S) {
        mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());
        uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
        nhash.resize(S + 1);
        for (int i = 0; i <= S; i++) {
            nhash[i] = dist(rng);
        }
    }

  private:
    static inline vector<uint64_t> nhash;
    void meet(int u, int v) { next[u] = v, prev[v] = u; }
    void meet(int u, int v, int w) { meet(u, v), meet(v, w); }
    void clr(int u) { prev[u] = next[u] = -1; }
};

// Maintain an incremental set of elements without duplicates in a vector. Very low memory overhead
template <typename T>
struct incremental_vector_set {
    static constexpr inline int B = 32;
    array<vector<T>, B> store = {};

    explicit incremental_vector_set(size_t reserved = 0) {
        for (int i = 0; i < B && reserved > 0; i++) {
            size_t here = min(reserved, 1ul << i);
            reserved -= here;
            store[i].reserve(1ul << i);
        }
    }

    bool has(const T& val) {
        for (int i = B - 1; i >= 0; i--) {
            if (binary_search(begin(store[i]), end(store[i]), val)) {
                return true;
            }
        }
        return false;
    }

    auto build() {
        int b = B - 1;
        while (b > 0 && store[b].empty()) {
            b--;
        }
        vector<size_t> sizes;
        for (int i = 0; i <= b; i++) {
            if (store[i].size()) {
                sizes.push_back(store[i].size());
            }
        }
        for (int i = b - 1; i >= 0; i--) {
            store[b].insert(end(store[b]), begin(store[i]), end(store[i]));
            store[i].clear();
            store[i].shrink_to_fit();
        }
        int C = sizes.size();
        size_t R = C ? sizes[0] : 0;
        for (int i = 1; i < C; i++) {
            size_t L = sizes[i];
            inplace_merge(end(store[b]) - L - R, end(store[b]) - R, end(store[b]));
            R += L;
        }
        assert(is_sorted(begin(store[b]), end(store[b])));
        return move(store[b]);
    }

    bool insert(const T& val) {
        if (has(val)) {
            return false;
        }
        int b = 0;
        while (store[b].size()) {
            b++;
        }
        for (int i = b - 1; i >= 0; i--) {
            store[b].insert(end(store[b]), begin(store[i]), end(store[i]));
            store[i].clear();
        }
        store[b].push_back(val);
        size_t K = 1;
        for (int i = 0; i < b; i++, K += K) {
            inplace_merge(end(store[b]) - 2 * K, end(store[b]) - K, end(store[b]));
        }
        assert(is_sorted(begin(store[b]), end(store[b])));
        return true;
    }
};
