#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T, typename Compare = less<>>
struct min_stack {
    vector<T> stack;
    Compare cmp;

    explicit min_stack(const Compare& cmp = Compare()) : cmp(cmp) {}

    auto pop() {
        T x = stack.back();
        return stack.pop_back(), x;
    }
    void push(T x) { stack.emplace_back(empty() ? x : min(x, top(), cmp)); }
    const auto& top() const { return stack.back(); }
    int size() const { return stack.size(); }
    bool empty() const { return stack.empty(); }
};

template <typename T, typename BinOp>
struct window_stack { // Aggregates (older, newer)
    vector<T> stack;
    BinOp binop;

    explicit window_stack(const BinOp& binop = BinOp()) : binop(binop) {}

    auto pop() {
        T x = stack.back();
        return stack.pop_back(), x;
    }
    void push(T x) { stack.emplace_back(empty() ? x : binop(top(), x)); }
    const auto& top() const { return stack.back(); }
    int size() const { return stack.size(); }
    bool empty() const { return stack.empty(); }
};
