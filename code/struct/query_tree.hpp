#pragma once

#include "algo/y_combinator.hpp"

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
        st.assign(2 * Q, {});
    }

    void insert(int L, int R, const E& element) {
        assert(0 <= L && L <= R && R <= N);
        if (L < R) {
            insert_recurse(1, 0, N, L, R, element);
        }
    }

    /**
     * -> Insert(E element), Save(), Rollback(), Answer(int time)
     * Save records the current state in a stack of changes/timestamps
     * Rollback() pops the stack of changes/timestamps once, back to the previous version
     */
    template <typename Insert, typename Save, typename Rollback, typename Answer>
    void visit(Insert&& insert, Save&& save, Rollback&& rollback, Answer&& answer) {
        y_combinator([&](auto self, int u, int l, int r) -> void {
            save();
            for (const auto& elem : st[u]) {
                insert(elem);
            }
            if (l + 1 == r) {
                answer(l);
            } else {
                int m = l + (r - l) / 2;
                self(u << 1, l, m);
                self(u << 1 | 1, m, r);
            }
            rollback();
        })(1, 0, N);
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

template <typename E, typename Insert, typename Rollback>
struct undo_stack_to_queue {
    Insert inserter;
    Rollback rollbacker;
    vector<pair<int, E>> stack; // A=1, B=0
    int A = 0, S = 0;

    explicit undo_stack_to_queue(const Insert& insert, const Rollback& rollback)
        : inserter(insert), rollbacker(rollback) {}

    void push(E elem) { stack.emplace_back(0, elem), S++, inserter(elem); }

    void pop() {
        if (stack.empty()) {
            return;
        } else if (stack.back().first == 1) {
            stack.pop_back(), A--, S--, rollbacker();
        } else if (A == 0) {
            for (int i = 0; i < S; i++) {
                rollbacker();
            }
            reverse(begin(stack), end(stack));
            for (int i = 0; i < S; i++) {
                stack[i].first = 1, A++;
                inserter(stack[i].second);
            }
            stack.pop_back(), A--, S--, rollbacker();
        } else {
            int D = 0; // Bs - As
            vector<pair<int, E>> popped[2] = {};
            do {
                int t = stack.back().first;
                popped[t].push_back(stack.back()), stack.pop_back();
                rollbacker();
                A -= t, D += t ? -1 : +1;
            } while (stack.size() && A > 0 && D > 0);
            while (popped[0].size()) {
                stack.push_back(popped[0].back()), popped[0].pop_back();
                inserter(stack.back().second);
            }
            while (popped[1].size()) {
                stack.push_back(popped[1].back()), popped[1].pop_back(), A++;
                inserter(stack.back().second);
            }
            stack.pop_back(), A--, S--, rollbacker();
        }
    }
};

template <typename E, typename Insert, typename Rollback>
auto make_undo_stack_to_queue(Insert&& insert, Rollback&& rollback) {
    return undo_stack_to_queue<E, Insert, Rollback>(insert, rollback);
}
