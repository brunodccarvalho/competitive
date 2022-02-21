#pragma once

#include "struct/persistent_queue.hpp"

/**
 * Persistent dueue with random access from the front or back in O(log N) time
 * Reference: https://publications.mpi-cbg.de/Myers_1983_6328.pdf
 * Note: cannot modify version 0 inplace (empty version)
 */
template <typename T>
struct persistent_jump_deque {
    persistent_jump_queue<T> fq, bq; // front queue and back queue

    persistent_jump_deque() = default;

    bool empty(int v) const { return fq.empty(v) && bq.empty(v); }
    int versions() const { return fq.versions(); }
    int size(int v) const { return fq.size(v) + bq.size(v); }

    T& front(int v) { return fq.empty(v) ? bq.front(v) : fq.back(v); }
    T& back(int v) { return bq.empty(v) ? fq.front(v) : bq.back(v); }

    int new_empty() { return fq.new_empty(), bq.new_empty(); }

    int clone(int v) { return fq.clone(v), bq.clone(v); }

    int pop_front(int v) {
        if (fq.empty(v)) {
            return fq.clone(v), bq.pop(v);
        } else {
            return bq.clone(v), fq.pop_back(v);
        }
    }

    int pop_back(int v) {
        if (bq.empty(v)) {
            return bq.clone(v), fq.pop(v);
        } else {
            return fq.clone(v), bq.pop_back(v);
        }
    }

    T& pop_front_inplace(int v) {
        if (fq.empty(v)) {
            return bq.pop_inplace(v);
        } else {
            return fq.pop_back_inplace(v);
        }
    }

    T& pop_back_inplace(int v) {
        if (bq.empty(v)) {
            return fq.pop_inplace(v);
        } else {
            return bq.pop_back_inplace(v);
        }
    }

    int push_front(int v, T element) { return bq.clone(v), fq.push(v, move(element)); }

    int push_back(int v, T element) { return fq.clone(v), bq.push(v, move(element)); }

    void push_front_inplace(int v, T element) {
        return fq.push_inplace(v, move(element));
    }

    void push_back_inplace(int v, T element) {
        return bq.push_inplace(v, move(element)); //
    }

    // 0 <= k < size(v), k=0 will give the front of the deque
    T& find_from_front(int v, int k) {
        if (int l = fq.size(v); k < l) {
            return fq.find_from_back(v, k);
        } else {
            return bq.find_from_front(v, k - l);
        }
    }

    // 0 <= k < size(v), k=0 will give the back of the deque
    T& find_from_back(int v, int k) {
        if (int l = bq.size(v); k < l) {
            return bq.find_from_back(v, k);
        } else {
            return fq.find_from_front(v, k - l);
        }
    }
};
