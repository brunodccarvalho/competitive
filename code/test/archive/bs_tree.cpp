#include "test_utils.hpp"
#include "../bstree/bs_map.hpp"
#include "../bstree/bs_set.hpp"
#include "../bstree/bs_tree_debug.hpp"

using namespace std;

#define ALL(set)  begin(set), end(set)
#define RALL(set) rbegin(set), rend(set)

/**
 * Non-copyable, non-movable, non-default-constructible type
 * that always allocates memory.
 */
struct always_allocs {
    always_allocs(int n1, int n2) : num1(new int(n1)), num2(new int(n2)) {}
    always_allocs(always_allocs&&) = delete;
    always_allocs(const always_allocs&) = delete;
    always_allocs& operator=(always_allocs&&) = delete;
    always_allocs& operator=(const always_allocs&) = delete;
    ~always_allocs() {
        assert(num1 && num2);
        delete num1;
        delete num2;
    }
    friend bool operator<(const always_allocs& lhs, const always_allocs& rhs) {
        return tie(*lhs.num1, *lhs.num2) < tie(*rhs.num1, *rhs.num2);
    }
    friend bool operator==(const always_allocs& lhs, const always_allocs& rhs) {
        return *lhs.num1 == *rhs.num1 && *lhs.num2 == *rhs.num2;
    }

  private:
    int *num1, *num2;
};

// Make sure these can be instantianed, at least...
template struct bs_set<int>;
template struct bs_set<pair<int, int>>;
template struct bs_set<int, greater<int>>;

/**
 * Test the following:
 *      - merge_unique(), merge_multi()
 *      - extract()
 *      - set_union, set_different, etc
 *      - bst_inserters
 */
void merge_test() {
    intd dists(0, 100);
    intd distn(-70, 70);

    // subtest 1: multi merge
    LOOP_FOR_DURATION_TRACKED (1s, now) {
        print_time(now, 1s, "multi merge test");

        bs_multiset<int> a, b;
        size_t as = dists(mt), bs = dists(mt);

        for (unsigned i = 0; i < as; i++) {
            a.insert(distn(mt));
        }
        for (unsigned i = 0; i < bs; i++) {
            b.insert(distn(mt));
        }
        assert(as == a.size() && bs == b.size());

        bs_multiset<int> uni, difab, difba, itr, sym;
        set_union(ALL(a), ALL(b), inserter(uni));
        set_difference(ALL(a), ALL(b), inserter(difab));
        set_difference(ALL(b), ALL(a), inserter(difba));
        set_intersection(ALL(a), ALL(b), inserter(itr));
        set_symmetric_difference(ALL(a), ALL(b), inserter(sym));

        a.merge(b);
        debug_tree(a).debug();
        debug_tree(b).debug();

        assert(a.size() == as + bs);
        assert(b.empty());

        assert(uni.size() == itr.size() + sym.size());
        assert(sym.size() == difab.size() + difba.size());
        debug_tree(uni).debug();
        debug_tree(difab).debug();
        debug_tree(difba).debug();
        debug_tree(sym).debug();
        debug_tree(itr).debug();

        // ping-pong
        bs_multiset<int> c = a;
        b.merge(c);
        c.merge(b);
        assert(a == c);
    }

    // subtest 2: unique merge
    LOOP_FOR_DURATION_TRACKED (1s, now) {
        print_time(now, 1s, "unique merge test");

        bs_set<int> a, b;
        size_t as = dists(mt), bs = dists(mt);

        for (unsigned i = 0; i < as; i++) {
            a.insert(distn(mt));
        }
        for (unsigned i = 0; i < bs; i++) {
            b.insert(distn(mt));
        }
        assert(as >= a.size() && bs >= b.size());
        as = a.size(), bs = b.size();

        bs_set<int> uni, difab, difba, itr, sym;
        set_union(ALL(a), ALL(b), inserter(uni));
        set_difference(ALL(a), ALL(b), inserter(difab));
        set_difference(ALL(b), ALL(a), inserter(difba));
        set_intersection(ALL(a), ALL(b), inserter(itr));
        set_symmetric_difference(ALL(a), ALL(b), inserter(sym));

        a.merge(b);
        debug_tree(a).debug();
        debug_tree(b).debug();
        assert(is_sorted(ALL(a)));
        assert(is_sorted(ALL(b)));

        assert(b.size() <= bs && as <= a.size());
        assert(a.size() + b.size() == as + bs);

        assert(uni.size() == a.size());
        assert(itr.size() == b.size());
        assert(uni.size() == itr.size() + sym.size());
        assert(sym.size() == difab.size() + difba.size());
        debug_tree(uni).debug();
        debug_tree(difab).debug();
        debug_tree(difba).debug();
        debug_tree(sym).debug();
        debug_tree(itr).debug();

        // ping-pong
        bs_set<int> c = a;
        b.merge(c);
        c.merge(b);
        debug_tree(a).debug();
        debug_tree(b).debug();
        debug_tree(c).debug();
        if (a != c) {
            print("\nTREE a:\n");
            debug_tree(a).pretty_print();
            print("\nTREE c:\n");
            debug_tree(c).pretty_print();
            vector<int> A(begin(a), end(a));
            vector<int> C(begin(c), end(c));
            print("A: {}\n", fmt::join(A, " "));
            print("C: {}\n", fmt::join(C, " "));
        }
        assert(a == c);
    }

    // subtest 3: extract
    LOOP_FOR_DURATION_TRACKED (1s, now) {
        print_time(now, 1s, "extract test");

        bs_multiset<int> a, b;
        int s = dists(mt);
        for (int i = 0; i < s; i++) {
            a.insert(distn(mt));
        }
        bs_multiset<int> c(a); // clone a
        vector<bs_multiset<int>::node_type> node_handles;
        for (auto it = a.begin(); it != a.end();) {
            auto nit = it;
            ++it;
            node_handles.push_back(a.extract(nit));
        }
        assert(a.empty());
        for (auto&& handle : node_handles) {
            b.insert(move(handle));
        }
        debug_tree(b).debug();
        assert(b == c);
    }
}

/**
 * Test the following:
 *      - Copy construction, move construction
 *      - Copy assignment, move assignment
 */
void construct_test() {
    intd dists(0, 50);
    intd distn(-1000, 1000);

    bs_multiset<int> d, e;

    LOOP_FOR_DURATION_TRACKED (1s, now) {
        print_time(now, 1s, "construct test");

        bs_multiset<int> a;
        for (int i = 0, s = dists(mt); i < s; i++) {
            a.insert(distn(mt));
        }

        // 1: copy construction
        bs_multiset<int> b(a);
        assert(a.size() == b.size() && a == b);

        // 2: move construction
        bs_multiset<int> c(move(a));
        assert(a.empty());
        assert(b.size() == c.size() && b == c);

        // 3: copy assignment
        d = b;
        assert(d.size() == b.size() && d == b);
        assert(d.size() == c.size() && d == c);

        // 4: move assignment
        e = move(b);
        assert(b.empty());
        assert(e.size() == c.size() && e == c);
        assert(e.size() == d.size() && e == d);

        debug_tree(a).debug();
        debug_tree(b).debug();
        debug_tree(c).debug();
        debug_tree(d).debug();
    }
}

/**
 * Test the following:
 *    1 - begin(), end() and friends
 *    2 - minimum(), maximum()
 *    3 - contains()
 *    4 - find(), count()
 *    5 - lower_bound(), upper_bound(), equal_range()
 */
void iterator_test() {
    intd dists(0, 50);
    intd distn(-10000, 10000);

    LOOP_FOR_DURATION_TRACKED (1s, now) {
        print_time(now, 1s, "iterator test");

        bs_multiset<int> tree;

        // generate a bunch of numbers, possibly repeated
        vector<int> nums;
        int s = dists(mt);
        for (int i = 0; i < s; i++) {
            nums.push_back(distn(mt));
        }
        // put them into the tree, ensure they were inserted
        for (int n : nums) {
            auto it = tree.insert(n);
            assert(it != tree.end() && *it == n);
        }
        sort(ALL(nums));
        assert(tree.size() == nums.size());
        debug_tree(tree).debug();

        // 1: test iteration
        assert(equal(ALL(nums), ALL(tree)));
        assert(equal(RALL(nums), RALL(tree)));
        assert(is_sorted(ALL(tree)));
        assert(is_sorted(RALL(tree), greater<int>{}));

        if (s == 0) {
            // 2: test minimum(), maximum()
            assert(tree.rbegin() == tree.rend());
            assert(tree.begin() == tree.end());
        } else {
            // 2: test minimum(), maximum()
            assert(*tree.begin() == nums.front());
            assert(*tree.rbegin() == nums.back());
        }

        // subtest 1: look for an existing integer n.
        for (int l = 0; s && l < 20; l++) {
            intd disti(0, s - 1);
            int i = disti(mt);
            int n = nums[i];

            auto nbegin = lower_bound(ALL(nums), n);
            auto nend = upper_bound(ALL(nums), n);
            auto ntotal = nend - nbegin;

            auto tbegin = tree.lower_bound(n);
            auto tend = tree.upper_bound(n);
            auto t2 = tree.equal_range(n);

            // 3: test contains(), because n actually exists
            assert(tree.contains(n));
            assert(tbegin != tend);
            // 4: test find(), count()
            assert(tree.find(n) != tree.end() && *tree.find(n) == n);
            assert(tree.count(n) == size_t(ntotal));
            assert(ntotal == distance(tbegin, tend));
            // 5: test equal_range(), should return the lower and upper bounds.
            assert(t2.first == tbegin && t2.second == tend);
            for (auto it = tbegin; it != tend; it++) {
                assert(*it == n);
            }
        }

        // subtest 2: look for the range between two existing integers n,m
        for (int l = 0; s && l < 30; l++) {
            intd disti(0, s - 1);
            int i = disti(mt), j = disti(mt);
            if (i > j)
                swap(i, j);
            int n = nums[i], m = nums[j];

            auto nbegin = lower_bound(ALL(nums), n);
            auto nend = upper_bound(ALL(nums), m);
            auto ntotal = nend - nbegin;

            auto tbegin = tree.lower_bound(n);
            auto tend = tree.upper_bound(m);

            // 3: test contains(), because n,m actually exist
            assert(tree.contains(n) && tree.contains(m));
            assert(tbegin != tend);
            // 4: test find()
            assert(tree.find(n) != tree.end() && *tree.find(n) == n);
            assert(tree.find(m) != tree.end() && *tree.find(m) == m);
            // 5: test lower_bound() and upper_bound()
            assert(ntotal == distance(tbegin, tend));
            assert(equal(tbegin, tend, nbegin, nend));
            for (auto it = tbegin; it != tend; it++) {
                assert(n <= *it && *it <= m);
            }
        }

        // subtest 3: look for an integer n that might, but probably doesn't exist
        for (int l = 0; l < 20; l++) {
            int n = distn(mt);

            auto nbegin = lower_bound(ALL(nums), n);
            auto nend = upper_bound(ALL(nums), n);
            auto ntotal = nend - nbegin;

            auto tbegin = tree.lower_bound(n);
            auto tend = tree.upper_bound(n);
            auto t2 = tree.equal_range(n);

            assert(tree.count(n) == size_t(ntotal));
            assert(ntotal == distance(tbegin, tend));
            // 5: test equal_range(), should return the lower and upper bounds.
            assert(t2.first == tbegin && t2.second == tend);
            for (auto it = tbegin; it != tend; it++) {
                assert(*it == n);
            }
        }

        // subtest 4: look for the range between two arbitrary integers n,m
        for (int l = 0; l < 20; l++) {
            int n = distn(mt), m = distn(mt);
            if (n > m)
                swap(n, m);

            auto nbegin = lower_bound(ALL(nums), n);
            auto nend = upper_bound(ALL(nums), m);
            auto ntotal = nend - nbegin;

            auto tbegin = tree.lower_bound(n);
            auto tend = tree.upper_bound(m);

            // 5: test lower_bound() and upper_bound()
            assert(ntotal == distance(tbegin, tend));
            assert(equal(tbegin, tend, nbegin, nend));
            for (auto it = tbegin; it != tend; it++) {
                assert(n <= *it && *it <= m);
            }
        }
    }
}

/**
 * Test the following:
 *      - comparison operators <=, >=, ==, !=
 *      - order invariance of elements (that the tree is actually sorted correctly)
 */
void equality_test() {
    intd dists(0, 200);
    intd distn(1, 10000);

    LOOP_FOR_DURATION_TRACKED (1s, now) {
        print_time(now, 1s, "equality test");

        bs_multiset<int> lhs, rhs;
        vector<int> nums;
        for (int i = 0, s = dists(mt); i < s; i++) {
            nums.push_back(distn(mt));
        }
        // insert into lhs in one order
        for (int n : nums) {
            lhs.insert(n);
        }
        // ... and into rhs in another
        shuffle(ALL(nums), mt);
        for (int n : nums) {
            rhs.insert(n);
        }
        debug_tree(lhs).debug();
        debug_tree(rhs).debug();

        sort(ALL(nums));

        assert(is_sorted(ALL(lhs)));
        assert(is_sorted(ALL(rhs)));
        assert(equal(ALL(lhs), ALL(nums)));
        assert(equal(ALL(rhs), ALL(nums)));
        assert(equal(ALL(lhs), ALL(rhs)));
        assert(lhs.size() == nums.size());
        assert(rhs.size() == nums.size());
        assert(lhs == rhs);
        assert(lhs <= rhs);
        assert(lhs >= rhs);
        assert(!(lhs < rhs));
        assert(!(lhs > rhs));
    }
}

/**
 * Test the following:
 *      - Comparison operators <, >
 */
void comparison_test(int T = 500) {
    intd dists(0, 100);
    intd distn(-500, 500);

    vector<bs_tree<int>> trees;
    vector<vector<int>> numsets;

    // generate many sets of nums and their corresponding trees.
    // sort the numsets and sort the trees, and ensure they were sorted the same.
    for (int t = 1; t <= T; t++) {
        bs_multiset<int> tree;
        vector<int> nums;
        for (int i = 0, s = dists(mt); i < s; i++) {
            nums.push_back(distn(mt));
        }
        for (int n : nums) {
            tree.insert(n);
        }
        trees.push_back(tree);
        sort(ALL(nums));
        numsets.push_back(nums);
    }

    sort(ALL(trees));
    sort(ALL(numsets));

    for (int i = 0; i < T; i++) {
        const auto& tree = trees[i];
        const auto& nums = numsets[i];
        debug_tree(tree).debug();
        assert(equal(ALL(tree), ALL(nums)));
    }
    for (int i = 0; i + 1 < T; i++) {
        assert(trees[i] <= trees[i + 1]);
        if (numsets[i] != numsets[i + 1]) {
            assert(trees[i] < trees[i + 1]);
            assert(trees[i] != trees[i + 1]);
        }
    }
}

/**
 * Test the following:
 *      - Tree core (avl/rb) insertion
 */
void insert_test() {
    intd distn(0, 1'000);
    intd dists(0, 200);

    LOOP_FOR_DURATION_TRACKED (1s, now) {
        print_time(now, 1s, "insert test");
        bs_set<int> tree;
        for (int i = 0, s = dists(mt); i < s; i++) {
            tree.insert(distn(mt));
            debug_tree(tree).debug();
        }
    }
}

/**
 * Test the following:
 *      - Tree core (avl/rb) removal
 */
void erase_test() {
    intd distn(0, 1'000);
    intd dists(0, 200);

    LOOP_FOR_DURATION_TRACKED (1s, now) {
        print_time(now, 1s, "erase test");
        bs_set<int> tree;
        vector<int> nums;
        int s = dists(mt);
        for (int i = 0; i < s; i++) {
            int n = distn(mt);
            nums.push_back(n);
            tree.insert(n);
        }
        debug_tree(tree).debug();
        shuffle(ALL(nums), mt);
        for (int i = 0; i < s; i++) {
            tree.erase(nums[i]);
            debug_tree(tree).debug();
        }
    }
}

/**
 * Test the following:
 *      - Value construction and destruction for non-trivial types
 *      - Memory allocation
 */
void memory_test() {
    bs_set<string> stree;

    string a(80, 'a'), b(40, 'b'), c(120, 'c'), d(200, 'd');
    stree.insert(c), stree.insert(b);
    stree.insert(a), stree.insert(d);

    debug_tree(stree).debug();
    assert(*stree.begin() == a);
    assert(*stree.rbegin() == d);

    bs_set<vector<int>> vtree;
    vector<int> v1(80, 1), v2(60, 2), v3(20, 3), v4(100, 4);
    vtree.insert(v3), vtree.insert(v2);
    vtree.insert(v1), vtree.insert(v4);

    debug_tree(vtree).debug();
    assert(*vtree.begin() == v1);
    assert(*vtree.rbegin() == v4);

    bs_set<always_allocs> atree;
    atree.emplace(3, 4), atree.emplace(2, 1);
    atree.emplace(2, 3), atree.emplace(1, 7);

    debug_tree(atree).debug();
    assert(*atree.begin() == always_allocs(1, 7));
    assert(*atree.rbegin() == always_allocs(3, 4));

    // remove everything and insert again, and check memory on valgrind
    stree.clear();
    vtree.clear();
    atree.clear();
    stree.insert(a), stree.insert(d);
    stree.insert(c), stree.insert(b);
    vtree.insert(v1), vtree.insert(v4);
    vtree.insert(v3), vtree.insert(v2);
    atree.emplace(2, 3), atree.emplace(1, 7);
    atree.emplace(3, 4), atree.emplace(2, 1);
}

/**
 * Test the following:
 *      - insert_hint_unique()  returned iterator
 *      - insert_hint_multi()  returned iterator
 *      - erase()
 */
void hint_test() {
    bs_tree<int> tree;

    auto it4 = tree.insert_unique(4).first;
    assert(*it4 == 4);

    // the first insert puts a 3, the following isnert before the initial 3.
    // 3 is inserted to the left of minimum()
    auto it3 = tree.insert_hint_unique(it4, 3);
    assert(tree.size() == 2);
    assert(it3 != tree.end() && *it3 == 3);
    auto it32 = tree.emplace_hint_unique(it3, 3);
    auto it33 = tree.insert_hint_unique(it4, 3);
    assert(tree.size() == 2);
    assert(it3 == it32 && it3 == it33);

    // the first insert puts a 5, the following ones put more 5 before the first one.
    // 5 is inserted to the right of maximum()
    auto it5 = tree.insert_hint_unique(it4, 5);
    assert(tree.size() == 3);
    assert(it5 != tree.end() && *it5 == 5);
    auto it52 = tree.emplace_hint_multi(it5, 5);
    auto it53 = tree.emplace_hint_multi(it52, 5);
    assert(tree.size() == 5);
    assert(*it52 == 5 && it52 != it5 && std::next(it52) == it5);
    assert(*it53 == 5 && it53 != it52 && std::next(it53) == it52);

    tree.erase(it52);
    tree.erase_unique(4);
    tree.erase(it3);
    tree.erase_unique(5);
    assert(!tree.empty());
    tree.erase_unique(5);
    assert(tree.empty());

    debug_tree(tree).debug();
}

/**
 * Test the following:
 *      - insert_unique() and emplace_unique() generate the same elements
 * Soft test - actual problems with emplace will be found in practice only
 */
void emplace_test() {
    bs_tree<pair<int, int>> a, b;

    a.insert_unique({3, 2});
    a.insert_unique({2, 3});
    a.insert_unique({1, 2});
    a.insert_unique({2, 1});
    a.insert_unique({1, 3});
    a.insert_unique({3, 1});

    b.emplace_unique(2, 1);
    b.emplace_unique(1, 3);
    b.emplace_unique(3, 1);
    b.emplace_unique(3, 2);
    b.emplace_unique(2, 3);
    b.emplace_unique(1, 2);

    a.emplace_multi(4, 7);
    a.emplace_multi(7, 4);
    a.emplace_multi(4, 7);

    b.insert_multi({4, 7});
    b.insert_multi({4, 7});
    b.insert_multi({7, 4});

    assert(a == b);
    debug_tree(a).debug();
    debug_tree(b).debug();
}

/**
 * Test the following:
 *      - bs_map's operator[], at()
 *      - bs_map's insert_or_assign(), try_emplace()
 */
void map_test() {
    bs_map<string, string> a, b;

    a["200"] = "def";
    a["300"] = "200";
    a["400"] = "300";
    a["200"] = "100";
    assert(a.size() == 3UL);

    b["400"] = "xyz";
    b["300"] = "200";
    b["200"] = "100";
    assert(b.size() == 3UL && a != b);
    b["400"] = "300";
    assert(b.size() == 3UL && a == b);
    assert(b.at("200") == "100");
    assert(b.at("400") == "300");

    b.clear();
    assert(b.empty());

    b.insert_or_assign("200", "300");
    b.insert_or_assign("300", "abc");
    b.insert_or_assign("300", "200");
    b.insert_or_assign("400", "def");
    b.insert_or_assign("400", "500");
    b.insert_or_assign("200", "100");
    assert(b.size() == 3UL && a != b);
    b.insert_or_assign("400", "300");
    assert(b.size() == 3UL && a == b);
    assert(b.at("200") == "100");
    assert(b.at("400") == "300");

    auto res = b.try_emplace("400", "xyz");
    assert(res.first != b.end() && !res.second);
    assert(b.size() == 3UL && a == b && b.at("400") == "300");
    res = b.try_emplace("500", "400");
    assert(res.first != b.end() && res.second);
    assert(b.size() == 4UL && a != b && b.at("500") == "400");

    a.emplace_hint(a.end(), "500", "400");
    assert(a == b);
}

void print_example() {
    intd distn(1, 99);
    bs_set<int> set;
    for (int i = 0; i < 15; i++) {
        set.insert(distn(mt));
    }
    debug_tree(set).pretty_print();
}

static int bti = 0;

/**
 * Test the following:
 *      - clear()
 *      - insert_unique(), insert_hint_unique()
 *      - insert_multi(), insert_hint_multi()
 *      - emplace_unique(), emplace_hint_unique()
 *      - emplace_multi(), emplace_hint_multi()
 *      - count(), find(), contains()
 * Compare the results of all operations with the equivalents ones on std::multiset.
 */
template <typename Compare = less<pair<int, int>>>
void battle_test(intd dists, intd distn, boold doerase, boold doemplace, boold domulti,
                 boold dohint, boold doclear) {
    bti++;

    using pair_t = pair<int, int>;
    using tree_t = bs_tree<pair_t, Compare>;
    using it_t = typename tree_t::const_iterator;
    using constit_t = typename tree_t::const_iterator;
    using stl_t = std::multiset<pair_t, Compare>;

    LOOP_FOR_DURATION_TRACKED (2s, now) {
        print_time(now, 2s, "battle test");

        tree_t tree;
        stl_t good;
        assert(tree.empty());

        int s = dists(mt);

        // probability of selecting an existing element for erasure
        boold doeraseexist(0.35);

        // iterator from the last insertion, for hinted insertions
        boold doreplaceit(0.4);
        constit_t prev_it = tree.end();

        for (int i = 0; i < s; i++) {
            int n1 = distn(mt), n2 = distn(mt);
            pair_t n = {n1, n2};
            size_t prev_count = tree.count(n);

            if (doerase(mt)) {
                if (!good.empty() && doeraseexist(mt)) {
                    auto goodit = good.begin();
                    advance(goodit, rand() % good.size());
                    n = *goodit;
                    prev_count = tree.count(n);
                }
                if (prev_it != tree.end() && *prev_it == n) {
                    prev_it = tree.end();
                }
                bool did_erase = tree.erase_unique(n);
                if (good.count(n))
                    good.erase(good.find(n));

                if (prev_count == 0) {
                    assert(!did_erase);
                    assert(tree.count(n) == 0);
                    assert(tree.find(n) == tree.end());
                } else {
                    assert(did_erase);
                    assert(tree.count(n) == prev_count - 1);
                }
            } else if (domulti(mt)) {
                it_t it;
                bool hinted = dohint(mt), emplaced = doemplace(mt);
                if (hinted && emplaced) {
                    it = tree.emplace_hint_multi(prev_it, n1, n2);
                } else if (hinted) {
                    it = tree.insert_hint_multi(prev_it, n);
                } else if (emplaced) {
                    it = tree.emplace_multi(n1, n2);
                } else {
                    it = tree.insert_multi(n);
                }
                good.insert(n);

                if (doreplaceit(mt)) {
                    prev_it = it;
                }

                assert(it != tree.end() && *it == n);
                assert(tree.contains(n));
                assert(tree.count(n) == prev_count + 1);
            } else {
                pair<it_t, bool> res;
                bool hinted = dohint(mt), emplaced = doemplace(mt);
                if (hinted && emplaced) {
                    res.second = tree.find(n) == tree.end();
                    res.first = tree.emplace_hint_unique(prev_it, n1, n2);
                } else if (hinted) {
                    res.second = tree.find(n) == tree.end();
                    res.first = tree.insert_hint_unique(prev_it, n);
                } else if (emplaced) {
                    res = tree.emplace_unique(n1, n2);
                } else {
                    res = tree.insert_unique(n);
                }
                if (!good.count(n))
                    good.insert(n);

                bool did_insert = res.second;
                auto it = res.first;
                if (doreplaceit(mt)) {
                    prev_it = it;
                }

                assert(it != tree.end() && *it == n);
                assert(tree.contains(n));
                if (prev_count == 0) {
                    assert(did_insert);
                    assert(tree.count(n) == 1);
                } else {
                    assert(!did_insert);
                    assert(tree.count(n) == prev_count);
                }
            }
            auto its = tree.equal_range(n);
            size_t dist = distance(its.first, its.second);
            assert(dist == good.count(n));
            assert(tree.count(n) == good.count(n));
            assert(tree.size() == good.size());

            debug_tree(tree).debug(); // verify invariants

            if (doclear(mt)) {
                tree.clear();
                good.clear();
                prev_it = tree.end();
            }
        }

        assert(is_sorted(ALL(tree), Compare{}));

        // validate all iterators, in both directions
        vector<pair_t> collector;
        for (auto it = tree.begin(); it != tree.end(); it++) {
            collector.push_back(*it);
        }
        assert(collector.size() == tree.size());
        assert(equal(ALL(collector), ALL(good)));
        collector.clear();
        for (auto it = tree.rbegin(); it != tree.rend(); it++) {
            collector.push_back(*it);
        }
        assert(collector.size() == tree.size());
        assert(equal(RALL(collector), ALL(good)));

        // punchline
        assert(equal(ALL(good), ALL(tree)));
    }
}

int main() {
    RUN_SHORT(insert_test());
    RUN_SHORT(erase_test());
    RUN_SHORT(hint_test());
    RUN_SHORT(emplace_test());
    RUN_SHORT(equality_test());
    RUN_SHORT(memory_test());
    RUN_SHORT(iterator_test());
    RUN_SHORT(construct_test());
    RUN_SHORT(comparison_test());
    RUN_SHORT(merge_test());
    RUN_SHORT(map_test());

    // clang-format off
    // generic test with moderate conflicts
    RUN_SHORT((battle_test(intd(5, 700), intd(0, 7),
                boold(0.40), boold(0.45), boold(0.30), boold(0.20), boold(0.002))));
    // inserts only test
    RUN_SHORT((battle_test(intd(5, 700), intd(0, 7),
                boold(0.00), boold(0.30), boold(0.60), boold(0.50), boold(0.000))));
    // generic test with many conflicts
    RUN_SHORT((battle_test(intd(100, 700), intd(0, 3),
                boold(0.40), boold(0.45), boold(0.30), boold(0.20), boold(0.003))));
    // hinted test
    RUN_SHORT((battle_test(intd(50, 200), intd(0, 7),
                boold(0.10), boold(0.50), boold(0.40), boold(0.90), boold(0.004))));
    // different compare
    RUN_SHORT((battle_test<greater<pair<int, int>>>(intd(5, 700), intd(0, 7),
                boold(0.40), boold(0.45), boold(0.30), boold(0.20), boold(0.002))));
    // many clears
    RUN_SHORT((battle_test(intd(200, 300), intd(0, 10),
                boold(0.10), boold(0.25), boold(0.50), boold(0.30), boold(0.075))));
    //               erase        emplace      multi        hint         clear
    // clang-format on

    print_example();

    return 0;
}
