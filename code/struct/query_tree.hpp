#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * For an associative and commutative data structure that allows true O(T(n)) insertion
 * add support for deletion in O(T(n)log(n)) offline.
 *
 * n is the time range. Queries insert or update the data structure in O(T(n)), and the
 * insert lives within a time range [l,r) after which it is deleted.
 *
 * This implementation queries the data structure at every timepoint.
 * Reference: https://github.com/Aeren1564/Algorithms/.../query_tree.sublime-snippet
 */
template <typename E>
struct query_tree {
    int N;
    vector<vector<E>> st;

    explicit query_tree(int N) : N(N) {
        int Q = 1 << (N > 1 ? 8 * sizeof(N) - __builtin_clz(N - 1) : 0);
        st.assign(2 * Q);
    }

    void insert(int L, int R, const E& element) {
        assert(0 <= L && L < R && R <= N);
        insert_recurse(1, 0, N, L, R, element);
    }

    /**
     * -> Insert(E element), Save(), Rollback(), Answer(int time)
     * Save records the current state in a stack of changes/timestamps
     * Rollback() pops the stack of changes/timestamps once, back to the previous version
     */
    template <typename Insert, typename Save, typename Rollback, typename Answer>
    void visit(Insert&& insert, Save&& save, Rollback&& rollback, Answer&& answer) {
        auto dfs = [&](const auto& dfs, int u, int l, int r) {
            save();
            for (const auto& elem : st[u]) {
                insert(elem);
            }
            if (l + 1 == r) {
                answer(l);
            } else {
                int m = l + (r - l) / 2;
                dfs(dfs, u << 1, l, m);
                dfs(dfs, u << 1 | 1, m, r);
            }
            rollback();
        };
        dfs(dfs, 1, 0, N);
    }

  private:
    void insert_recurse(int u, int l, int r, int L, int R, const E& element) {
        if (L <= l && r <= R) {
            st[u].push_back(element);
            return;
        }
        int m = l + (r - l) / 2;
        if (R <= m) {
            insert_recurse(u << 1, l, m, L, R, element);
        } else if (m <= L) {
            insert_recurse(u << 1 | 1, m, r, L, R, element);
        } else {
            insert_recurse(u << 1, l, m, L, m, element);
            insert_recurse(u << 1 | 1, m, r, m, R, element);
        }
    }
};
