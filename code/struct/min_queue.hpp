#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T, typename Compare = less<T>>
struct min_queue {
    vector<T> fs, bs; // front stack, back stack
    T recent;
    Compare cmp;

    explicit min_queue(const Compare& cmp = Compare()) : cmp(cmp) {}

    auto pop() {
        T x = top();
        transfer(), fs.pop_back();
        return x;
    }
    void push(T x) { recent = bs.empty() ? x : min(recent, x, cmp), bs.push_back(x); }
    auto top() const {
        return bs.empty() ? fs.back() : fs.empty() ? recent : min(fs.back(), recent, cmp);
    }
    int size() const { return fs.size() + bs.size(); }
    bool empty() const { return fs.empty() && bs.empty(); }

  private:
    void transfer() {
        if (fs.empty()) {
            while (!bs.empty()) {
                T x = bs.back();
                bs.pop_back();
                fs.push_back(fs.empty() ? x : min(x, fs.back(), cmp));
            }
        }
    }
};

template <typename T, typename BinOp>
struct window_queue { // Aggregates (older, newer)
    vector<T> fs, bs; // front stack, back stack
    T recent;
    BinOp binop;

    explicit window_queue(const BinOp& binop = BinOp()) : binop(binop) {}

    auto pop() {
        T x = top();
        transfer(), fs.pop_back();
        return x;
    }
    void push(T x) { recent = bs.empty() ? x : binop(recent, x), bs.push_back(x); }
    auto top() const {
        return bs.empty() ? fs.back() : fs.empty() ? recent : binop(fs.back(), recent);
    }
    int size() const { return fs.size() + bs.size(); }
    bool empty() const { return fs.empty() && bs.empty(); }

  private:
    void transfer() {
        if (fs.empty()) {
            while (!bs.empty()) {
                T x = bs.back();
                bs.pop_back();
                fs.push_back(fs.empty() ? x : binop(x, fs.back()));
            }
        }
    }
};
