#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T, typename Compare = less<T>>
struct min_deque {
    vector<T> fd, bd; // front/back stacks of unmerged data
    vector<T> fs, bs; // front/back stacks of aggregates
    Compare cmp;

    explicit min_deque(const Compare& cmp = Compare()) : cmp(cmp) {}

    auto pop_front() {
        T old_window = window();
        if (!fs.empty()) { // just pop from front
            fs.pop_back(), fd.pop_back();
        } else if (bs.size() <= 1u) { // just clear
            clear();
        } else { // move half from back to front, rebuilding everything
            int S = bs.size(), F = S / 2;
            fs.clear(), bs.clear();
            for (int i = F; i >= 1; i--) {
                fs.push_back(fs.empty() ? bd[i] : min(bd[i], fs.back(), cmp));
            }
            for (int i = F + 1; i < S; i++) {
                bs.push_back(bs.empty() ? bd[i] : min(bs.back(), bd[i], cmp));
            }
            fd.assign(begin(bd) + 1, begin(bd) + F + 1);
            bd.erase(begin(bd), begin(bd) + F + 1);
            reverse(begin(fd), end(fd));
        }
        return old_window;
    }

    auto pop_back() {
        T old_window = window();
        if (!bs.empty()) { // just pop from back
            bs.pop_back(), bd.pop_back();
        } else if (fs.size() <= 1u) { // just clear
            clear();
        } else { // move half from front to back, rebuilding everything
            int S = fs.size(), F = S / 2;
            fs.clear(), bs.clear();
            for (int i = F; i >= 1; i--) {
                bs.push_back(bs.empty() ? fd[i] : min(bs.back(), fd[i], cmp));
            }
            for (int i = F + 1; i < S; i++) {
                fs.push_back(fs.empty() ? fd[i] : min(fd[i], fs.back(), cmp));
            }
            bd.assign(begin(fd) + 1, begin(fd) + F + 1);
            fd.erase(begin(fd), begin(fd) + F + 1);
            reverse(begin(bd), end(bd));
        }
        return old_window;
    }

    void push_front(T x) {
        fs.push_back(fs.empty() ? x : min(x, fs.back(), cmp)), fd.push_back(x);
    }
    void push_back(T x) {
        bs.push_back(bs.empty() ? x : min(bs.back(), x, cmp)), bd.push_back(x);
    }

    auto window() const {
        if (bs.empty()) {
            return fs.empty() ? T() : fs.back();
        } else {
            return fs.empty() ? bs.back() : min(fs.back(), bs.back(), cmp);
        }
    }

    int size() const { return fs.size() + bs.size(); }
    bool empty() const { return fs.empty() && bs.empty(); }
    void clear() { fd.clear(), bd.clear(), fs.clear(), bs.clear(); }
};

template <typename T, typename BinOp, typename A = T>
struct window_deque { // Aggregates (front, back) (A|T,A|T)->A, T is data, A is aggregate
    vector<T> fd, bd; // front/back stacks of unmerged data
    vector<A> fs, bs; // front/back stacks of aggregates
    BinOp binop;

    explicit window_deque(const BinOp& binop = BinOp()) : binop(binop) {}

    auto pop_front() {
        T old_window = window();
        if (!fs.empty()) { // just pop from front
            fs.pop_back(), fd.pop_back();
        } else if (bs.size() <= 1u) { // just clear
            clear();
        } else { // move half from back to front, rebuilding everything
            int S = bs.size(), F = S / 2;
            fs.clear(), bs.clear();
            for (int i = F; i >= 1; i--) {
                fs.push_back(fs.empty() ? bd[i] : binop(bd[i], fs.back()));
            }
            for (int i = F + 1; i < S; i++) {
                bs.push_back(bs.empty() ? bd[i] : binop(bs.back(), bd[i]));
            }
            fd.assign(begin(bd) + 1, begin(bd) + F + 1);
            bd.erase(begin(bd), begin(bd) + F + 1);
            reverse(begin(fd), end(fd));
        }
        return old_window;
    }

    auto pop_back() {
        T old_window = window();
        if (!bs.empty()) { // just pop from back
            bs.pop_back(), bd.pop_back();
        } else if (fs.size() <= 1u) { // just clear
            clear();
        } else { // move half from front to back, rebuilding everything
            int S = fs.size(), F = S / 2;
            fs.clear(), bs.clear();
            for (int i = F; i >= 1; i--) {
                bs.push_back(bs.empty() ? fd[i] : binop(bs.back(), fd[i]));
            }
            for (int i = F + 1; i < S; i++) {
                fs.push_back(fs.empty() ? fd[i] : binop(fd[i], fs.back()));
            }
            bd.assign(begin(fd) + 1, begin(fd) + F + 1);
            fd.erase(begin(fd), begin(fd) + F + 1);
            reverse(begin(bd), end(bd));
        }
        return old_window;
    }

    void push_front(T x) {
        fs.push_back(fs.empty() ? x : binop(x, fs.back())), fd.push_back(x);
    }
    void push_back(T x) {
        bs.push_back(bs.empty() ? x : binop(bs.back(), x)), bd.push_back(x);
    }

    auto window() const {
        if (bs.empty()) {
            return fs.empty() ? T() : fs.back();
        } else {
            return fs.empty() ? bs.back() : binop(fs.back(), bs.back());
        }
    }

    int size() const { return fs.size() + bs.size(); }
    bool empty() const { return fs.empty() && bs.empty(); }
    void clear() { fd.clear(), bd.clear(), fs.clear(), bs.clear(); }
};
