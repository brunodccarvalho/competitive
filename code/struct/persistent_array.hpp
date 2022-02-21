#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Fat nodes implementation, partially persistent
 *
 * Complexity: O(1) set, O(log U) access where U is the number of writes on the index
 */
template <typename T>
struct persistent_fat_array {
    vector<T> data;
    int N, V;
    vector<vector<pair<int, int>>> fat;

    template <typename... Args>
    explicit persistent_fat_array(Args&&... args)
        : data(forward<Args>(args)...), N(data.size()), V(1), fat(N) {
        for (int i = 0; i < N; i++) {
            fat[i].emplace_back(0, i);
        }
    }

    int num_nodes() const { return data.size(); }
    int versions() const { return V; }

    int set(int i, T element) {
        fat[i].emplace_back(V, num_nodes());
        data.push_back(move(element));
        return V++;
    }

    T& get(int v, int i) {
        auto at = upper_bound(begin(fat[i]), end(fat[i]), make_pair(v, num_nodes()));
        return data[prev(at)[1]];
    }
};
