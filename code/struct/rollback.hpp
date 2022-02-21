#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T, typename... Rs>
struct multiset_rollback : multiset<T, Rs...> {
    using base_t = multiset<T, Rs...>;

    enum Operation { INSERT, ERASE };
    vector<pair<Operation, T>> history;

    template <typename... Args>
    explicit multiset_rollback(Args&&... args) : base_t(forward<Args>(args)...) {}

    int time() const { return history.size(); }

    void rollback(int t) {
        int i = time();
        while (i-- > t) {
            const auto& [op, value] = history.back();
            if (op == INSERT) {
                base_t::erase(base_t::find(value));
            } else if (op == ERASE) {
                base_t::insert(value);
            }
            history.pop_back();
        }
    }

    void insert(const T& value) {
        history.emplace_back(INSERT, value);
        base_t::insert(value);
    }

    void erase(const T& value) {
        if (auto it = base_t::find(value); it != base_t::end()) {
            history.emplace_back(ERASE, value);
            base_t::erase(it);
        }
    }

    void clear() {
        for (const auto& value : *this) {
            history.emplace_back(ERASE, value);
        }
        base_t::clear();
    }
};

template <typename Key, typename Value, typename... Rs>
struct map_rollback : map<Key, Value, Rs...> {
    using base_t = map<Key, Value, Rs...>;

    enum Operation { INSERT, MODIFY, ERASE };
    vector<tuple<Operation, Key, Value>> history;

    template <typename... Args>
    explicit map_rollback(Args&&... args) : base_t(forward<Args>(args)...) {}

    int time() const { return history.size(); }

    void rollback(int t) {
        int i = time();
        while (i-- > t) {
            const auto& [op, key, value] = history.back();
            if (op == INSERT) {
                base_t::erase(key);
            } else if (op == ERASE) {
                base_t::emplace(key, value);
            } else if (op == MODIFY) {
                base_t::find(key)->second = value;
            }
            history.pop_back();
        }
    }

    Value& operator[](const Key& key) {
        if (auto it = base_t::find(key); it != base_t::end()) {
            history.emplace_back(MODIFY, key, it->second);
            return it->second;
        } else {
            history.emplace_back(INSERT, key, Value());
            return base_t::emplace(key, Value()).first->second;
        }
    }

    Value& at(const Key& key) {
        const auto& kv = base_t::at(key);
        history.emplace_back(MODIFY, key, kv.second);
        return kv.second;
    }

    const Value& at(const Key& key) const {
        const auto& kv = base_t::at(key);
        return kv.second;
    }

    const Value& get(const Key& key) const {
        const auto& kv = base_t::at(key);
        return kv.second;
    }

    void erase(const Key& key) {
        if (auto it = base_t::find(key); it != base_t::end()) {
            history.emplace_back(ERASE, it->first, it->second);
            base_t::erase(it);
        }
    }

    void clear() {
        for (const auto& value : *this) {
            history.emplace_back(ERASE, value);
        }
        base_t::clear();
    }
};

template <typename T>
struct vector_rollback : vector<T> {
    using base_t = vector<T>;

    vector<pair<int, T>> history;

    template <typename... Args>
    explicit vector_rollback(Args&&... args) : base_t(forward<Args>(args)...) {}

    int time() const { return history.size(); }
    void rollback(int t) {
        int i = time();
        while (i-- > t) {
            rollback();
        }
    }
    void rollback() {
        auto [i, value] = history.back();
        base_t::at(i) = value;
        history.pop_back();
    }

    void set(int i, T value) {
        history.emplace_back(i, base_t::at(i));
        base_t::at(i) = value;
    }
};
