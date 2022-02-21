#pragma once

#include <bits/stdc++.h>
using namespace std;

// min cartesian tree for less<T> (minimum on the root), lefts go higher
template <typename T, typename Compare = less<T>>
auto cartesian_tree(const vector<T>& arr, const Compare& cmp = Compare()) {
    int N = arr.size();
    vector<int> parent(N, -1);
    vector<array<int, 2>> kids(N, {-1, -1});

    for (int i = 1; i < N; i++) {
        int p = i - 1;
        while (parent[p] != -1 && cmp(arr[i], arr[p])) {
            p = parent[p];
        }
        if (cmp(arr[i], arr[p])) {
            parent[i] = parent[p], kids[i][0] = p;
        } else {
            parent[i] = p, kids[i][0] = kids[p][1];
        }
        if (parent[i] != -1) {
            kids[parent[i]][1] = i;
        }
        if (kids[i][0] != -1) {
            parent[kids[i][0]] = i;
        }
    }

    int root = N - 1;
    while (parent[root] != -1)
        root = parent[root];

    return make_tuple(root, move(parent), move(kids));
}
