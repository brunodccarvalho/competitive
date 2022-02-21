#include "test_utils.hpp"
#include "struct/treap.hpp"

auto operator+(deque<int> a, const deque<int>& b) {
    a.insert(end(a), begin(b), end(b));
    return a;
}

auto rand_treap() {
    int key = rand_unif<int>(0, INT_MAX);
    int value = rand_unif<int>(0, 1'000'000);
    return new Treap(key, value);
}

void stress_test_treap_order() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 1000, runs) {
        print_time(now, 20s, "stress treap order ({} runs)", runs);

        const int MAX = 150;
        int N = 0;
        deque<int> arr;
        Treap* root = nullptr;

        for (int loop = 0; loop < 20'000; loop++) {
            if (cointoss(0.20) && N > 0) { // * pop_back
                root = delete_back(root);
                arr.pop_back(), N--;
            }
            if (cointoss(0.95) && N < MAX) { // * push_back
                auto node = rand_treap();
                root = push_back(root, node);
                arr.push_back(node->key), N++;
            }
            if (cointoss(0.20) && N > 0) { // * pop_front
                root = delete_front(root);
                arr.pop_front(), N--;
            }
            if (cointoss(0.95) && N < MAX) { // * push_front
                auto node = rand_treap();
                root = push_front(root, node);
                arr.push_front(node->key), N++;
            }
            if (cointoss(0.75) && N < MAX) { // * insert_order
                int order = rand_unif<int>(0, N);
                auto node = rand_treap();
                root = insert_order(root, node, order);
                arr.insert(begin(arr) + order, node->key), N++;
            }
            if (cointoss(0.4) && N > 0) { // * delete_order
                int order = rand_unif<int>(0, N - 1);
                root = delete_order(root, order);
                arr.erase(begin(arr) + order), N--;
            }
            if (cointoss(0.2)) { // * rotate with split_order
                int order = rand_unif<int>(0, N);
                auto [a, b] = split_order(root, order);
                root = join(b, a);
                rotate(begin(arr), begin(arr) + order, end(arr));
            }
            if (cointoss(0.8) && N > 0) { // * find_order
                int order = rand_unif<int>(0, N - 1);
                auto node = find_order(root, order);
                assert(node && node->key == arr[order]);
            }
            if (cointoss(0.05) && N > 0) { // * delete_order_range
                auto [a, b] = ordered_unif<int>(0, N);
                root = delete_order_range(root, a, b);
                arr.erase(begin(arr) + a, begin(arr) + b), N -= b - a;
            }
            if (cointoss(0.25)) { // * split_order
                deque<int> inorder, ai, bi;
                visit_inorder(root, [&](Treap* u) { inorder.push_back(u->key); });
                assert(inorder == arr);

                int x = rand_unif<int>(0, N);
                auto [a, b] = split_order(root, x);

                visit_inorder(a, [&](Treap* u) { ai.push_back(u->key); });
                visit_inorder(b, [&](Treap* u) { bi.push_back(u->key); });

                assert(int(ai.size()) == x);
                assert(int(bi.size()) == N - x);
                assert(inorder == ai + bi);

                root = join(a, b);
            }
            if (cointoss(0.15)) { // * split_order_range
                deque<int> inorder, ai, bi, ci;
                visit_inorder(root, [&](Treap* u) { inorder.push_back(u->key); });
                assert(inorder == arr);

                auto [x, y] = ordered_unif<int>(0, N);
                auto [a, b, c] = split_order_range(root, x, y);

                visit_inorder(a, [&](Treap* u) { ai.push_back(u->key); });
                visit_inorder(b, [&](Treap* u) { bi.push_back(u->key); });
                visit_inorder(c, [&](Treap* u) { ci.push_back(u->key); });

                assert(int(ai.size()) == x);
                assert(int(bi.size()) == y - x);
                assert(int(ci.size()) == N - y);
                assert(inorder == ai + bi + ci);

                root = join(a, b, c);
            }
        }
    }
}

void stress_test_treap_key() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 1000, runs) {
        print_time(now, 20s, "stress treap key ({} runs)\r", runs);

        const int MAX = 150;
        int N = 0;
        deque<int> arr;
        Treap* root = nullptr;

        auto any_key = []() { return rand_unif<int>(0, INT_MAX); };
        auto rand_key = [&]() {
            return N == 0 ? any_key() : arr[rand_unif<int>(0, N - 1)];
        };
        auto rand_keys = [&]() {
            static int a = rand_key(), b = rand_key();
            return a <= b ? pair<int, int>{a, b} : pair<int, int>{b, a};
        };

        for (int loop = 0; loop < 20'000; loop++) {
            if (cointoss(0.75) && N < MAX) { // insert by order
                auto node = rand_treap();
                root = insert_key(root, node);
                arr.insert(lower_bound(begin(arr), end(arr), node->key), node->key), N++;
            }
            if (cointoss(0.4) && N > 0) { // delete by key
                int key = rand_key();
                root = delete_key(root, key);
                arr.erase(lower_bound(begin(arr), end(arr), key)), N--;
            }
            if (cointoss(0.8) && N > 0) { // find by key valid
                int key = rand_key();
                auto node = find_key(root, key);
                assert(node && node->key == key);
            }
            if (cointoss(0.8)) { // after <==> lower_bound
                int key = any_key();
                auto node = after(root, key);
                int i = lower_bound(begin(arr), end(arr), key) - begin(arr);
                assert(i < N ? node && node->key == arr[i] : !node);
            }
            if (cointoss(0.8)) { // strict_after <==> upper_bound
                int key = any_key();
                auto node = strict_after(root, key);
                int i = upper_bound(begin(arr), end(arr), key) - begin(arr);
                assert(i < N ? node && node->key == arr[i] : !node);
            }
            if (cointoss(0.8)) { // strict_before <==> lower_bound - 1
                int key = any_key();
                auto node = strict_before(root, key);
                int i = int(lower_bound(begin(arr), end(arr), key) - begin(arr)) - 1;
                assert(i >= 0 ? node && node->key == arr[i] : !node);
            }
            if (cointoss(0.8)) { // before <==> upper_bound - 1
                int key = any_key();
                auto node = before(root, key);
                int i = int(upper_bound(begin(arr), end(arr), key) - begin(arr)) - 1;
                assert(i >= 0 ? node && node->key == arr[i] : !node);
            }
            if (cointoss(0.05) && N > 0) { // delete range by key
                auto [x, y] = rand_keys();
                root = delete_key_range(root, x, y);
                arr.erase(lower_bound(begin(arr), end(arr), x),
                          lower_bound(begin(arr), end(arr), y));
                N = arr.size();
            }
            if (cointoss(0.25)) { // split and join by key
                deque<int> inorder, ai, bi;
                visit_inorder(root, [&](Treap* u) { inorder.push_back(u->key); });
                assert(is_sorted(begin(inorder), end(inorder)) && inorder == arr);

                int k = rand_key();
                auto [a, b] = split_key(root, k);

                visit_inorder(a, [&](Treap* u) {
                    ai.push_back(u->key), assert(u->key < k);
                });
                visit_inorder(b, [&](Treap* u) {
                    bi.push_back(u->key), assert(k <= u->key);
                });
                assert(inorder == ai + bi);

                root = join(a, b);
            }
            if (cointoss(0.15)) { // split and join by key three ways
                deque<int> inorder, ai, bi, ci;
                visit_inorder(root, [&](Treap* u) { inorder.push_back(u->key); });
                assert(is_sorted(begin(inorder), end(inorder)) && inorder == arr);

                int x, y;
                tie(x, y) = rand_keys();
                auto [a, b, c] = split_key_range(root, x, y);

                visit_inorder(a, [&](Treap* u) {
                    ai.push_back(u->key), assert(u->key < x);
                });
                visit_inorder(b, [&](Treap* u) {
                    bi.push_back(u->key), assert(x <= u->key && u->key < y);
                });
                visit_inorder(c, [&](Treap* u) {
                    ci.push_back(u->key), assert(y <= u->key);
                });
                assert(inorder == ai + bi + ci);

                root = join(a, b, c);
            }
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_treap_order());
    RUN_BLOCK(stress_test_treap_key());
    return 0;
}
