#include "test_utils.hpp"
#include "struct/functional_treap.hpp"

using STreap = shared_ptr<Treap>;

auto operator+(deque<int> a, const deque<int>& b) {
    a.insert(end(a), begin(b), end(b));
    return a;
}

constexpr int64_t MAXKEY = 100'000'000;

auto any_key() { return rand_unif<int64_t>(0, MAXKEY); }

auto arr_key(const deque<int>& arr) { return arr[rand_unif<int>(0, arr.size() - 1)]; }

auto key_range() { return different<int>(0, MAXKEY); }

auto rand_treap() { return make_shared<Treap>(any_key()); }

auto ordered(int L, int R) {
    int a = rand_unif<int>(L, R), b = rand_unif<int>(L, R);
    return a <= b ? array<int, 2>{a, b} : array<int, 2>{b, a};
}

void unit_test_functional_treap() {
    auto a = make_shared<Treap>(10);
    auto b = make_shared<Treap>(20);
    auto c = make_shared<Treap>(30);

    auto d = join(a, b, c);

    putln(format_inorder(d));

    auto e = insert_key(d, make_shared<Treap>(35));
    auto f = insert_key(e, make_shared<Treap>(22));
    auto g = insert_key(f, make_shared<Treap>(27));
    auto h = insert_key(g, make_shared<Treap>(15));
    auto i = insert_key(h, make_shared<Treap>(11));
    auto j = insert_key(i, make_shared<Treap>(4));
    auto k = insert_key(j, make_shared<Treap>(8));
    auto l = insert_key(k, make_shared<Treap>(13));
    auto m = insert_key(l, make_shared<Treap>(1));
    auto n = insert_key(m, make_shared<Treap>(32));

    putln(format_inorder(e));
    putln(format_inorder(f));
    putln(format_inorder(g));
    putln(format_inorder(h));
    putln(format_inorder(i));
    putln(format_inorder(j));
    putln(format_inorder(k));
    putln(format_inorder(l));
    putln(format_inorder(m));
    putln(format_inorder(n));
}

void stress_test_functional_treap_order() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 1000, runs) {
        print_time(now, 20s, "stress functional treap order ({} runs)", runs);

        const int MAX = 150;
        int N = 0;
        deque<int> arr;
        STreap root;

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
            if (cointoss(0.2)) { // * split_order rotate
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
                visit_inorder(root, [&](STreap u) { inorder.push_back(u->key); });
                assert(inorder == arr);

                int x = rand_unif<int>(0, N);
                auto [a, b] = split_order(root, x);

                visit_inorder(a, [&](STreap u) { ai.push_back(u->key); });
                visit_inorder(b, [&](STreap u) { bi.push_back(u->key); });

                assert(int(ai.size()) == x);
                assert(int(bi.size()) == N - x);
                assert(inorder == ai + bi);

                root = join(a, b);
            }
            if (cointoss(0.15)) { // * split_order_range
                deque<int> inorder, ai, bi, ci;
                visit_inorder(root, [&](STreap u) { inorder.push_back(u->key); });
                assert(inorder == arr);

                auto [x, y] = ordered_unif<int>(0, N);
                auto [a, b, c] = split_order_range(root, x, y);

                visit_inorder(a, [&](STreap u) { ai.push_back(u->key); });
                visit_inorder(b, [&](STreap u) { bi.push_back(u->key); });
                visit_inorder(c, [&](STreap u) { ci.push_back(u->key); });

                assert(int(ai.size()) == x);
                assert(int(bi.size()) == y - x);
                assert(int(ci.size()) == N - y);
                assert(inorder == ai + bi + ci);

                root = join(a, b, c);
            }
        }
    }
}

int main() {
    RUN_BLOCK(unit_test_functional_treap());
    RUN_BLOCK(stress_test_functional_treap_order());
    return 0;
}
