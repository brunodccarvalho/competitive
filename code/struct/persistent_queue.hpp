#pragma once

#include "struct/persistent_stack.hpp"

/**
 * Simple implementation with two stacks
 */
template <typename T>
struct persistent_queue_stacks {
    persistent_stack<T> inbox, outbox;

    persistent_queue_stacks() = default;

    bool empty(int v) const { return inbox.empty(v) && outbox.empty(v); }
    int size(int v) const { return inbox.size(v) + outbox.size(v); }
    int versions() const { return inbox.versions(); }

    T& front(int v) { return flush(v), outbox.top(v); }

    int new_empty() { return inbox.new_empty(), outbox.new_empty(); }

    int clone(int v) { return inbox.clone(v), outbox.clone(v); }

    int pop(int v) {
        inbox.clone(v), v = outbox.clone(v);
        flush(v), outbox.pop_inplace(v);
        return v;
    }

    T& pop_inplace(int v) {
        flush(v);
        return outbox.pop_inplace(v);
    }

    int push(int v, T element) {
        inbox.push(v, move(element));
        return outbox.clone(v);
    }

    void push_inplace(int v, T element) {
        inbox.push_inplace(v, move(element)); //
    }

  private:
    void flush(int v) {
        if (outbox.empty(v)) {
            while (!inbox.empty(v)) {
                outbox.push_inplace(v, inbox.pop_inplace(v));
            }
        }
    }
};

/**
 * Persistent queue with random access from the front or back in O(log N) time
 * Reference: https://publications.mpi-cbg.de/Myers_1983_6328.pdf
 * Note: cannot modify version 0 inplace (empty version)
 */
template <typename T>
struct persistent_jump_queue {
    struct Node {
        const int parent, jump, depth;
        T data;
    };
    vector<Node> node;
    vector<array<int, 2>> ends;

    persistent_jump_queue() : node(1, {0, 0, 0, T()}), ends(1, {0, 0}) {}

    bool empty(int v) const { return ends[v][1] == 0; }
    int versions() const { return ends.size(); }
    int size(int v) const {
        return node[ends[v][1]].depth - node[node[ends[v][0]].parent].depth;
    }

    T& front(int v) { return node[ends[v][0]].data; }
    T& back(int v) { return node[ends[v][1]].data; }

    int new_empty() {
        ends.push_back({0, 0});
        return ends.size() - 1;
    }

    int clone(int v) {
        ends.push_back(ends[v]);
        return ends.size() - 1;
    }

    // pop element from the front of the queue
    int pop(int v) {
        auto [head, tail] = ends[v];
        if (head == tail) {
            return clone(0);
        }
        int new_head = search(tail, node[head].depth + 1);
        ends.push_back({new_head, tail});
        return ends.size() - 1;
    }

    // pop element from the front of the queue, modify version
    T& pop_inplace(int v) {
        assert(v != 0);
        auto& [head, tail] = ends[v];
        T& data = node[head].data;
        if (head == tail) {
            head = tail = 0;
            return data;
        }
        head = search(tail, node[head].depth + 1);
        return data;
    }

    // push element to the back of the queue
    int push(int v, T element) {
        auto [head, tail] = ends[v];
        int t = node[tail].jump, l = node[tail].depth, j = node[t].jump;
        if (l - node[t].depth == node[t].depth - node[j].depth) {
            t = j;
        } else {
            t = tail;
        }
        int new_tail = node.size();
        int new_head = head ? head : new_tail;
        ends.push_back({new_head, new_tail});
        node.push_back({tail, t, 1 + l, move(element)});
        return ends.size() - 1;
    }

    // push element to the back of the queue, modify version
    void push_inplace(int v, T element) {
        assert(v != 0);
        auto& [head, tail] = ends[v];
        int t = node[tail].jump, l = node[tail].depth, j = node[t].jump;
        if (l - node[t].depth == node[t].depth - node[j].depth) {
            t = j;
        } else {
            t = tail;
        }
        node.push_back({tail, t, 1 + l, move(element)});
        tail = node.size() - 1;
        head = head ? head : tail;
    }

    // pop element from the back of the queue (most recent)
    int pop_back(int v) {
        auto [head, tail] = ends[v];
        if (head == tail)
            return clone(0);
        ends.push_back({head, node[tail].parent});
        return ends.size() - 1;
    }

    // pop element from the back of the queue (most recent), modify version
    T& pop_back_inplace(int v) {
        assert(v != 0);
        auto& [head, tail] = ends[v];
        T& data = node[tail].data;
        if (head == tail) {
            head = tail = 0;
            return data;
        }
        tail = node[tail].parent;
        return data;
    }

    // 0 <= k < length[v], k=0 will give oldest element in the queue (front)
    T& find_from_front(int v, int k) {
        return node[search(ends[v][1], node[ends[v][0]].depth + k)].data;
    }

    // 0 <= k < length[v], k=0 will give newest element in the stack (back)
    T& find_from_back(int v, int k) {
        return node[search(ends[v][1], node[ends[v][1]].depth - k)].data;
    }

  private:
    int search(int u, int depth) {
        assert(0 <= depth && depth <= node[u].depth);
        while (node[u].depth != depth) {
            if (node[node[u].jump].depth < depth) {
                u = node[u].parent;
            } else {
                u = node[u].jump;
            }
        }
        return u;
    }
};
