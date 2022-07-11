#pragma once

#include "struct/integer_lists.hpp"

template <typename T>
struct circular_deque {
    explicit circular_deque(int N) : data(N), N(N) {}

    void push_back(T elem) { S++, data[j] = move(elem), j = j + 1 == N ? 0 : j + 1; }
    void push_front(T elem) { S++, i = i == 0 ? N - 1 : i - 1, data[i] = move(elem); }

    T pop_front() { return S--, i == N - 1 ? i = 0, data[N - 1] : data[i++]; }
    T pop_back() { return S--, j == 0 ? j = N - 1, data[0] : data[j--]; }

    bool empty() const { return S == 0; }
    int size() const { return S; }
    int universe() const { return N; }
    void clear() { i = j = S = 0; }

  private:
    vector<T> data;
    int N = 0, i = 0, j = 0, S = 0;
};

template <typename T>
struct circular_queue {
    explicit circular_queue(int N) : data(N), N(N) {}

    void push(T elem) { S++, data[j] = move(elem), j = j + 1 == N ? 0 : j + 1; }

    T pop() { return S--, i == N - 1 ? i = 0, data[N - 1] : data[i++]; }

    bool empty() const { return S == 0; }
    int size() const { return S; }
    int universe() const { return N; }
    void clear() { i = j = S = 0; }

  private:
    vector<T> data;
    int N = 0, i = 0, j = 0, S = 0;
};

template <typename Compare>
struct spfa_deque {
    explicit spfa_deque(int N, const Compare& cmp = Compare())
        : cmp(cmp), in(N), data(N), N(N) {}

    void push(int u) {
        if (!in[u]) {
            if (S > 0 && !cmp(data[i], u) && coind(rng)) {
                i = i == 0 ? N - 1 : i - 1, data[i] = u; // push to front
            } else {
                data[j] = u, j = j + 1 == N ? 0 : j + 1; // push to back
            }
            S++, in[u] = true;
        }
    }
    int pop() {
        int u = data[i];
        S--, in[u] = false, i = i == N - 1 ? 0 : i + 1;
        return u;
    }

    bool empty() const { return S == 0; }
    int size() const { return S; }
    int universe() const { return N; }
    void clear() { i = j = S = 0; }

    static inline mt19937 rng = mt19937(random_device{}());
    static inline bernoulli_distribution coind = bernoulli_distribution(0.73);

  private:
    Compare cmp;
    vector<bool> in;
    vector<int> data;
    int N = 0, i = 0, j = 0, S = 0;
};

struct buckets_queue {
    int N = 0, B = 0, S = 0, M = 0;
    linked_lists ll;

    explicit buckets_queue(int N, int B) : N(N), B(B) { clear(); }

    void push(int u, int bucket) {
        assert(S <= bucket && bucket < B);
        if (ll.next[u] != u) {
            ll.erase(u);
        }
        ll.push_back(bucket, u);
        M = max(M, bucket + 1);
    }

    int top() {
        while (S < M && ll.empty(S)) {
            S++;
        }
        return S == M ? -1 : ll.head(S);
    }

    int pop() {
        while (S < M && ll.empty(S)) {
            S++;
        }
        if (S == M) {
            return -1;
        } else {
            int u = ll.head(S);
            ll.pop_front(S);
            ll.wrap(u);
            return u;
        }
    }

    void clear() {
        S = 0, ll.assign(B, N);
        for (int i = 0; i < N; i++) {
            ll.wrap(i);
        }
    }

    bool empty() { return top() == -1; }
    int universe() const { return N; }
    int buckets() const { return B; }
};
