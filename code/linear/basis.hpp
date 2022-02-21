#pragma once

#include <bits/stdc++.h>
using namespace std;

// Representation of a vector space basis. Vectors need not have consistent dimension
// Reduction is O(dim B), does not support exchange queries
template <typename T>
struct vector_space_basis {
    using V = vector<T>;
    list<V> basis;

    int dimensions() const { return basis.empty() ? 0 : basis.front().size(); }
    int basis_size() const { return basis.size(); }
    void clear() { basis.clear(); }

    auto reduce(V vec) { return operate(vec, false); }

    // Return true if the vector is representable (possibly empty)
    bool representable(const V& vec) { return operate(vec, false).empty(); }

    // Return true if the vector is not empty and can be/was added to the basis
    bool check(const V& vec) { return !operate(vec, false).empty(); }
    bool insert_check(const V& vec) { return !operate(vec, true).empty(); }

  private:
    // Reduce and maybe insert the vec in basis. Returns the vector orderly reduced
    auto operate(V vec, bool add_basis) {
        while (!vec.empty() && vec.back() == T()) {
            vec.pop_back();
        }
        for (auto it = begin(basis); it != end(basis) && !vec.empty(); ++it) {
            if (it->size() < vec.size()) {
                if (add_basis) {
                    basis.insert(it, vec);
                }
                return vec;
            } else if (it->size() == vec.size()) {
                auto ratio = vec.back() / it->back();
                for (int j = vec.size() - 1; j >= 0; j--) {
                    vec[j] -= ratio * it->at(j);
                }
                while (!vec.empty() && vec.back() == T()) {
                    vec.pop_back();
                }
            }
        }
        if (add_basis && !vec.empty()) {
            basis.insert(end(basis), vec);
        }
        return vec;
    }
};

// Representation of a binary xor basis with ints.
// Reduction is O(B), does not support exchange queries
template <typename T>
struct integer_xor_basis {
    vector<T> basis;

    int dimensions() const { return basis.empty() ? 0 : basis.front().size(); }
    int basis_size() const { return basis.size(); }
    void clear() { basis.clear(); }

    auto reduce(T vec) { return operate(vec, false); }

    // Return true if the vector is representable (possibly empty)
    bool representable(const T& vec) { return operate(vec, false) == T(); }

    // Return true if the vector is not empty and can be/was added to the basis
    bool check(const T& vec) { return operate(vec, false) != T(); }
    bool insert_check(const T& vec) { return operate(vec, true) != T(); }

  private:
    // Reduce and maybe insert the vec in basis. Returns the vector fully reduced
    auto operate(T vec, bool add_basis) {
        for (int i = 0, B = basis_size(); i < B && vec != T(); i++) {
            vec = min(vec, vec ^ basis[i]);
        }
        if (add_basis && vec) {
            basis.push_back(vec);
        }
        return vec;
    }
};

// Representation of a binary xor basis with bitsets
// Reduction is O(SB), does not support exchange queries
template <int S>
struct bitset_xor_basis {
    using V = bitset<S>;
    list<pair<int, V>> basis;

    int dimensions() const { return basis.empty() ? 0 : basis.front().first; }
    int basis_size() const { return basis.size(); }
    void clear() { basis.clear(); }

    auto reduce(V vec) { return operate(vec, false); }

    // Return true if the vector is representable (possibly empty)
    bool representable(const V& vec) { return operate(vec, false).none(); }

    // Return true if the vector is not empty and can be/was added to the basis
    bool check(const V& vec) { return operate(vec, false).any(); }
    bool insert_check(const V& vec) { return operate(vec, true).any(); }

  private:
    // Reduce and maybe insert the vec in basis. Returns the vector orderly reduced
    auto operate(V vec, bool add_basis) {
        int c = S - 1;
        while (c >= 0 && !vec[c]) {
            c--;
        }
        for (auto it = begin(basis); it != end(basis) && c >= 0; ++it) {
            if (it->first < c) {
                if (add_basis) {
                    basis.insert(it, make_pair(c, vec));
                }
                return vec;
            } else if (it->first == c) {
                vec ^= it->second;
                while (c >= 0 && !vec[c]) {
                    c--;
                }
            }
        }
        if (add_basis && c >= 0) {
            basis.push_back(make_pair(c, vec));
        }
        return vec;
    }
};
