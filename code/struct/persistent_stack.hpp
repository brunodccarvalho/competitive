#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Simple implementation based on a tree with no jump pointers for random access
 * Complexity: O(1) push, O(1) amortized pop
 * Note: cannot modify version 0 inplace. top/pop on empty stack has no effect.
 */
template <typename T>
struct persistent_stack {
    struct Node {
        const int next, length;
        T data;
    };
    vector<Node> node;
    vector<int> heads;

    persistent_stack() : node(1, {0, 0, T()}), heads(1, 0) {}

    bool empty(int v) const { return heads[v] == 0; }
    int size(int v) const { return node[heads[v]].length; }
    int versions() const { return heads.size(); }

    T& top(int v) { return node[heads[v]].data; }

    int new_empty() {
        heads.push_back(0);
        return heads.size() - 1;
    }

    int clone(int v) {
        heads.push_back(heads[v]);
        return heads.size() - 1;
    }

    int pop(int v) {
        heads.push_back(node[heads[v]].next);
        return heads.size() - 1;
    }

    int push(int v, T element) {
        heads.push_back(node.size());
        node.push_back({heads[v], 1 + size(v), move(element)});
        return heads.size() - 1;
    }

    T& pop_inplace(int v) {
        assert(v != 0);
        T& data = node[heads[v]].data;
        heads[v] = node[heads[v]].next;
        return data;
    }

    void push_inplace(int v, T element) {
        assert(v != 0);
        node.push_back({heads[v], 1 + size(v), move(element)});
        heads[v] = node.size() - 1;
    }
};

/**
 * Persistent stack with random access from the bottom or top in O(log N) time
 * Reference: https://publications.mpi-cbg.de/Myers_1983_6328.pdf
 * Note: cannot modify version 0 inplace. top/pop on empty stack has no effect.
 */
template <typename T>
struct persistent_jump_stack {
    struct Node {
        const int next, jump, length;
        T data;
    };
    vector<Node> node;
    vector<int> heads;

    persistent_jump_stack() : node(1, {0, 0, 0, T()}), heads(1, 0) {}

    bool empty(int v) const { return heads[v] == 0; }
    int size(int v) const { return node[heads[v]].length; }
    int versions() const { return heads.size(); }

    T& top(int v) { return node[heads[v]].data; }

    int new_empty() {
        heads.push_back(0);
        return heads.size() - 1;
    }

    int clone(int v) {
        heads.push_back(heads[v]);
        return heads.size() - 1;
    }

    int pop(int v) {
        heads.push_back(node[heads[v]].next);
        return heads.size() - 1;
    }

    int push(int v, T element) {
        int head = heads[v];
        int t = node[head].jump, l = node[head].length, j = node[t].jump;
        if (l - node[t].length == node[t].length - node[j].length) {
            t = j;
        } else {
            t = head;
        }
        heads.push_back(node.size());
        node.push_back({head, t, 1 + l, move(element)});
        return heads.size() - 1;
    }

    T& pop_inplace(int v) {
        T& data = node[heads[v]].data;
        heads[v] = node[heads[v]].next;
        return data;
    }

    void push_inplace(int v, T element) {
        int& head = heads[v];
        int t = node[head].jump, l = node[head].length, j = node[t].jump;
        if (l - node[t].length == node[t].length - node[j].length) {
            t = j;
        } else {
            t = head;
        }
        node.push_back({head, t, 1 + l, move(element)});
        head = node.size() - 1;
    }

    // 0 <= k < length[v], k=0 will give oldest element in the stack
    T& find_from_bottom(int v, int k) { return node[search(heads[v], k + 1)].data; }

    // 0 <= k < length[v], k=0 will give newest element in the stack (top)
    T& find_from_top(int v, int k) { return node[search(heads[v], size(v) - k)].data; }

  private:
    int search(int u, int length) {
        assert(0 <= length && length <= node[u].length);
        while (node[u].length != length) {
            if (node[node[u].jump].length < length) {
                u = node[u].next;
            } else {
                u = node[u].jump;
            }
        }
        return u;
    }
};
