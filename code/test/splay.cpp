#include "test_utils.hpp"
#include "struct/splay.hpp"

auto operator+(deque<int> a, const deque<int>& b) {
    a.insert(end(a), begin(b), end(b));
    return a;
}

constexpr int64_t MAXKEY = 100'000'000;

auto any_key() { return rand_unif<int64_t>(0, MAXKEY); }

auto arr_key(const deque<int>& arr) { return arr[rand_unif<int>(0, arr.size() - 1)]; }

auto key_range() { return different<int>(0, MAXKEY); }

auto rand_splay() { return new Splay(any_key()); }

auto ordered(int L, int R) {
    int a = rand_unif<int>(L, R), b = rand_unif<int>(L, R);
    return a <= b ? array<int, 2>{a, b} : array<int, 2>{b, a};
}

void stress_test_splay_order() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 1000, runs) {
        print_time(now, 20s, "stress splay order ({} runs)", runs);
        const int MAX = 200;
        int N = 0;
        deque<int> arr;
        Splay* tree = nullptr;

        for (int loop = 0; loop < 20'000; loop++) {
            if (cointoss(0.95)) { // * range update
                int add = rand_unif<int>(-100, 100);
                auto [a, b] = ordered(0, N);
                if (auto node = access_order_range(tree, a, b)) {
                    node->update_range(add);
                    tree = splay(node);
                    for (int i = a; i < b; i++) {
                        arr[i] += add;
                    }
                }
            }
            if (cointoss(0.95) && N > 0) { // * unit update
                int add = rand_unif<int>(-100, 100);
                int i = rand_unif<int>(0, N - 1);
                find_order(tree, i)->update_self(add);
                arr[i] += add;
            }

            if (cointoss(0.20) && N > 0) { // * pop_back
                delete_back(tree);
                arr.pop_back(), N--;
            }
            if (cointoss(0.95) && N < MAX) { // * push_back
                auto node = rand_splay();
                push_back(tree, node);
                arr.push_back(node->key), N++;
            }
            if (cointoss(0.20) && N > 0) { // * pop_front
                delete_front(tree);
                arr.pop_front(), N--;
            }
            if (cointoss(0.95) && N < MAX) { // * push_front
                auto node = rand_splay();
                push_front(tree, node);
                arr.push_front(node->key), N++;
            }
            if (cointoss(0.20) && N > 0) { // * delete_order
                int order = rand_unif<int>(-1, N + 1);
                delete_order(tree, order);
                if (0 <= order && order < N) {
                    arr.erase(begin(arr) + order), N--;
                }
            }
            if (cointoss(0.95) && N < MAX) { // * insert_order
                int order = rand_unif<int>(-1, N + 1);
                auto node = rand_splay();
                insert_order(tree, node, order);
                order = clamp(order, 0, N);
                arr.insert(begin(arr) + order, node->key), N++;
            }
            if (cointoss(0.30) && N > 0) { // * splice_order into push_back
                int order = rand_unif<int>(0, N - 1);
                auto node = splice_order(tree, order);
                push_back(tree, node);
                arr.push_back(arr[order]), arr.erase(begin(arr) + order);
            }
            if (cointoss(0.30) && N > 0) { // * splice_order into push_front
                int order = rand_unif<int>(0, N - 1);
                auto node = splice_order(tree, order);
                push_front(tree, node);
                arr.push_front(arr[order]), arr.erase(begin(arr) + order + 1);
            }
            if (cointoss(0.30) && N > 1) { // * splice_item into insert_after
                auto [a, b] = different<int>(0, N - 1);
                int c = rand_unif<int>(0, N - 1);
                if (cointoss(0.5)) {
                    swap(a, b);
                }

                auto A = find_order(tree, a);
                auto B = find_order(tree, b);
                assert(A && B && A != B);
                find_order(tree, c);
                splice_item(tree, A);
                insert_after(tree, B, A);
                if (a < b) {
                    arr.insert(begin(arr) + b + 1, A->key);
                    arr.erase(begin(arr) + a);
                } else {
                    arr.erase(begin(arr) + a);
                    arr.insert(begin(arr) + b + 1, A->key);
                }
            }
            if (cointoss(0.20) && N > 0) { // * delete_item
                int order = rand_unif<int>(0, N - 1);
                int other = rand_unif<int>(0, N - 1);
                auto node = find_order(tree, order);
                find_order(tree, other);
                delete_item(tree, node);
                if (order < N) {
                    arr.erase(begin(arr) + order), N--;
                }
            }
            if (cointoss(0.30)) { // * rotate with split_order
                int order = rand_unif<int>(0, N);
                auto [a, b] = split_order(tree, order);
                tree = join(b, a);
                rotate(begin(arr), begin(arr) + order, end(arr));
            }

            if (cointoss(0.75)) { // * access_order_range
                auto [a, b] = ordered(0, N);
                auto node = access_order_range(tree, a, b);
                auto sum = node ? node->sum : 0;
                auto ans = accumulate(begin(arr) + a, begin(arr) + b, 0LL);
                assert(sum == ans);
            }
            if (cointoss(0.30)) { // * splice_order_range into back
                auto [a, b] = ordered(0, N);
                auto range = splice_order_range(tree, a, b);
                tree = join(tree, range);
                vector<int> nums(begin(arr) + a, begin(arr) + b);
                arr.insert(end(arr), begin(nums), end(nums));
                arr.erase(begin(arr) + a, begin(arr) + b);
            }
            if (cointoss(0.30)) { // * splice_order_range into front
                auto [a, b] = ordered(0, N);
                auto range = splice_order_range(tree, a, b);
                tree = join(range, tree);
                vector<int> nums(begin(arr) + a, begin(arr) + b);
                arr.erase(begin(arr) + a, begin(arr) + b);
                arr.insert(begin(arr), begin(nums), end(nums));
            }
            if (cointoss(0.07)) { // * delete_order_range
                auto [a, b] = ordered(0, N);
                delete_order_range(tree, a, b);
                arr.erase(begin(arr) + a, begin(arr) + b), N -= b - a;
            }
            if (cointoss(0.25)) { // * split_order and join 2
                deque<int> inorder, ai, bi;
                visit_inorder(tree, [&](Splay* u) { inorder.push_back(u->key); });
                assert(inorder == arr);

                int x = rand_unif<int>(0, N);
                auto [a, b] = split_order(tree, x);

                visit_inorder(a, [&](Splay* u) { ai.push_back(u->key); });
                visit_inorder(b, [&](Splay* u) { bi.push_back(u->key); });

                assert(int(ai.size()) == x);
                assert(int(bi.size()) == N - x);
                assert(inorder == ai + bi);

                tree = join(a, b);
            }
            if (cointoss(0.15)) { // * split_order_range and join 3
                deque<int> inorder, ai, bi, ci;
                visit_inorder(tree, [&](Splay* u) { inorder.push_back(u->key); });
                assert(inorder == arr);

                auto [x, y] = ordered(0, N);
                auto [a, b, c] = split_order_range(tree, x, y);

                visit_inorder(a, [&](Splay* u) { ai.push_back(u->key); });
                visit_inorder(b, [&](Splay* u) { bi.push_back(u->key); });
                visit_inorder(c, [&](Splay* u) { ci.push_back(u->key); });

                assert(int(ai.size()) == x);
                assert(int(bi.size()) == y - x);
                assert(int(ci.size()) == N - y);
                assert(inorder == ai + bi + ci);

                tree = join(a, b, c);
            }
            if (cointoss(0.30) && N > 0) { // * back()
                assert(back(tree)->key == arr.back());
            }
            if (cointoss(0.30) && N > 0) { // * front()
                assert(front(tree)->key == arr.front());
            }
            if (cointoss(0.30) && N > 0) { // * find_order()
                int order = rand_unif<int>(0, N - 1);
                assert(find_order(tree, order)->key == arr[order]);
            }
            if (cointoss(0.30) && N > 1) { // * predecessor(back())
                assert(predecessor(tree, back(tree))->key == arr[N - 2]);
            }
            if (cointoss(0.30) && N > 1) { // * successor(front())
                assert(successor(tree, front(tree))->key == arr[1]);
            }

            assert(!tree || !tree->parent);
            assert(!tree || tree->size == N);

            deque<int> inorder;
            visit_inorder(tree, [&](Splay* u) { inorder.push_back(u->key); });
            assert(inorder == arr);
        }
    }
}

void stress_test_splay_key() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 1000, runs) {
        print_time(now, 20s, "stress splay key ({} runs)", runs);
        const int MAX = 200;
        int N = 0;
        deque<int> arr;
        Splay* tree = nullptr;

        auto get = [&](int key) { return lower_bound(begin(arr), end(arr), key); };

        for (int loop = 0; loop < 20'000; loop++) {
            if (cointoss(0.20) && N > 0) { // * pop_back
                delete_back(tree);
                arr.pop_back(), N--;
            }
            if (cointoss(0.20) && N > 0) { // * pop_front
                delete_front(tree);
                arr.pop_front(), N--;
            }
            if (cointoss(0.20) && N > 0) { // * delete_key
                int key = arr_key(arr);
                delete_key(tree, key);
                arr.erase(get(key)), N--;
            }
            if (cointoss(0.95) && N < MAX) { // * insert_key
                auto node = rand_splay();
                insert_key(tree, node);
                arr.insert(get(node->key), node->key), N++;
            }
            if (cointoss(0.20) && N > 0) { // * delete_item from key
                int key = arr_key(arr);
                auto node = find_key(tree, key);
                delete_item(tree, node);
                arr.erase(get(key)), N--;
            }
            if (cointoss(0.25)) { // * access_key_range
                auto [a, b] = key_range();
                auto node = access_key_range(tree, a, b);
                auto sum = node ? node->sum : 0;
                auto ans = accumulate(get(a), get(b), 0LL);
                assert(sum == ans);
            }
            if (cointoss(0.30)) { // * splice_key_range into insert_key
                auto [a, b] = key_range();
                auto range = splice_key_range(tree, a, b);
                insert_key(tree, range);
            }
            if (cointoss(0.07) && N > 0) { // * delete_key_range
                auto [a, b] = key_range();
                delete_key_range(tree, a, b);
                N -= get(b) - get(a);
                arr.erase(get(a), get(b));
            }
            if (cointoss(0.25)) { // * split_order and join 2
                deque<int> inorder, ai, bi;
                visit_inorder(tree, [&](Splay* u) { inorder.push_back(u->key); });
                assert(inorder == arr);

                int x = any_key();
                auto [a, b] = split_key(tree, x);

                visit_inorder(a, [&](Splay* u) { ai.push_back(u->key); });
                visit_inorder(b, [&](Splay* u) { bi.push_back(u->key); });

                assert(inorder == ai + bi);

                for (int an : ai)
                    assert(an < x);
                for (int bn : bi)
                    assert(x <= bn);

                tree = join(a, b);
            }
            if (cointoss(0.15)) { // * split_key_range and join 3
                deque<int> inorder, ai, bi, ci;
                visit_inorder(tree, [&](Splay* u) { inorder.push_back(u->key); });
                assert(inorder == arr);

                auto [x, y] = key_range();
                auto [a, b, c] = split_key_range(tree, x, y);

                visit_inorder(a, [&](Splay* u) { ai.push_back(u->key); });
                visit_inorder(b, [&](Splay* u) { bi.push_back(u->key); });
                visit_inorder(c, [&](Splay* u) { ci.push_back(u->key); });

                assert(inorder == ai + bi + ci);

                for (int an : ai)
                    assert(an < x);
                for (int bn : bi)
                    assert(x <= bn && bn < y);
                for (int cn : ci)
                    assert(y <= cn);

                tree = join(a, b, c);
            }
            if (cointoss(0.30) && N > 0) { // * back()
                assert(back(tree)->key == arr.back());
            }
            if (cointoss(0.30) && N > 0) { // * front()
                assert(front(tree)->key == arr.front());
            }
            if (cointoss(0.30) && N > 0) { // * find_key()
                int key = arr_key(arr);
                assert(find_key(tree, key)->key == key);
            }
            if (cointoss(0.30) && N > 1) { // * predecessor(back())
                assert(predecessor(tree, back(tree))->key == arr[N - 2]);
            }
            if (cointoss(0.30) && N > 1) { // * successor(front())
                assert(successor(tree, front(tree))->key == arr[1]);
            }
            if (cointoss(0.30)) { // * after == lower_bound
                int key = any_key();
                auto node = after(tree, key);
                auto it = lower_bound(begin(arr), end(arr), key);
                assert(!node == (it == end(arr)));
                assert(!node || node->key == *it);
            }
            if (cointoss(0.30)) { // * strict_after == upper_bound
                int key = any_key();
                auto node = strict_after(tree, key);
                auto it = upper_bound(begin(arr), end(arr), key);
                assert(!node == (it == end(arr)));
                assert(!node || node->key == *it);
            }
            if (cointoss(0.30)) { // * before == upper_bound - 1 == rev lower_bound
                int key = any_key();
                auto node = before(tree, key);
                auto it = lower_bound(rbegin(arr), rend(arr), key, greater<int>{});
                assert(!node == (it == rend(arr)));
                assert(!node || node->key == *it);
            }
            if (cointoss(0.30)) { // * strict_before == lower_bound - 1 == rev upper_bound
                int key = any_key();
                auto node = strict_before(tree, key);
                auto it = upper_bound(rbegin(arr), rend(arr), key, greater<int>{});
                assert(!node == (it == rend(arr)));
                assert(!node || node->key == *it);
            }

            if (cointoss(0.03) && N < MAX) { // * meld two splays
                int G = (MAX - N + 1) / 2;
                Splay* other = nullptr;
                for (int i = 0; i < G; i++) {
                    auto node = rand_splay();
                    insert_key(other, node);
                    arr.push_back(node->key);
                }
                sort(begin(arr) + N, end(arr));
                inplace_merge(begin(arr), begin(arr) + N, end(arr)), N += G;
                tree = cointoss(0.5) ? meld(tree, other) : meld(other, tree);
            }

            assert(!tree || !tree->parent);
            assert(!tree || tree->size == N);

            deque<int> inorder;
            visit_inorder(tree, [&](Splay* u) { inorder.push_back(u->key); });
            assert(inorder == arr);
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_splay_order());
    RUN_BLOCK(stress_test_splay_key());
    return 0;
}
