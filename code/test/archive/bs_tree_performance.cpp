#define NDEBUG
#include "test_utils.hpp"
#include "../bstree/bs_map.hpp"
#include "../bstree/bs_set.hpp"

using namespace std;

template struct bs_set<int>;
template struct bs_set<int, greater<int>>;
template struct bs_multiset<int>;
template struct bs_multiset<int, greater<int>>;
template struct std::set<int>;
template struct std::set<int, greater<int>>;
template struct std::multiset<int>;
template struct std::multiset<int, greater<int>>;

inline namespace bstree_performance_testers {

struct unordered_insert_test {
    template <typename Set>
    static void run(const string& name, int M, int T, int f) {
        intd distn(0, M);

        START(test);
        size_t size = 0;
        for (int t = 0; t < T; t++) {
            Set set;
            for (int i = 0; i < f * t; i++) {
                set.insert(distn(mt));
            }
            size += set.size();
        }
        TIME(test);

        double avg_size = 1.0 * size / T;
        print(" {:>12} {:6}ms  (size: {:.1f})\n", name, TIME_MS(test), avg_size);
    }
};

struct ordered_insert_hint_begin_test {
    template <typename Set>
    static void run(const string& name, int M, int T, int f) {
        intd distn(0, M);
        vector<int> nums(T * f);
        for (int i = 0; i < T * f; i++) {
            nums[i] = distn(mt);
        }
        sort(begin(nums), end(nums), greater<int>{});

        START(test);
        size_t size = 0;
        for (int t = 0; t < T; t++) {
            Set set;
            for (int i = 0; i < f * t; i++) {
                assert(!i || nums[i] <= *set.begin());
                set.insert(set.begin(), nums[i]);
            }
            size += set.size();
        }
        TIME(test);

        double avg_size = 1.0 * size / T;
        print(" {:>12} {:6}ms  (size: {:.1f})\n", name, TIME_MS(test), avg_size);
    }
};

struct ordered_insert_hint_end_test {
    template <typename Set>
    static void run(const string& name, int M, int T, int f) {
        intd distn(0, M);
        vector<int> nums(T * f);
        for (int i = 0; i < T * f; i++) {
            nums[i] = distn(mt);
        }
        sort(begin(nums), end(nums));

        START(test);
        size_t size = 0;
        for (int t = 0; t < T; t++) {
            Set set;
            for (int i = 0; i < f * t; i++) {
                assert(!i || *set.rbegin() <= nums[i]);
                set.insert(set.end(), nums[i]);
            }
            size += set.size();
        }
        TIME(test);

        double avg_size = 1.0 * size / T;
        print(" {:>12} {:6}ms  (size: {:.1f})\n", name, TIME_MS(test), avg_size);
    }
};

struct count_query_test {
    template <typename Set>
    static void run(const string& name, int M, int T, int f, int q) {
        intd distn(0, M);
        vector<Set> sets(T);
        for (int t = 0; t < T; t++) {
            for (int i = 0; i < f * t; i++) {
                sets[t].insert(distn(mt));
            }
        }

        START(test);
        size_t count = 0;
        for (int t = 0; t < T; t++) {
            for (int i = 0; i < q * t; i++) {
                count += sets[t].count(distn(mt));
            }
        }
        TIME(test);

        print(" {:>12} {:6}ms  (size: {})\n", name, TIME_MS(test), count);
    }
};

struct random_erase_test {
    template <typename Set>
    static void run(const string& name, int M, int T, int f, int e) {
        intd distn(0, M);
        vector<Set> sets(T);
        for (int t = 0; t < T; t++) {
            for (int i = 0; i < f * t; i++) {
                sets[t].insert(distn(mt));
            }
        }

        START(test);
        size_t count = 0;
        for (int t = 0; t < T; t++) {
            for (int i = 0; i < e * t; i++) {
                count += sets[t].erase(distn(mt));
            }
        }
        TIME(test);

        print(" {:>12} {:6}ms  (size: {})\n", name, TIME_MS(test), count);
    }
};

struct perfect_unordered_erase_test {
    template <typename Set>
    static void run(const string& name, int M, int T, int f) {
        intd distn(0, M);
        vector<vector<int>> nums(T);
        vector<Set> sets(T);
        for (int t = 0; t < T; t++) {
            nums[t].resize(f * t);
            for (int i = 0; i < f * t; i++) {
                int n = distn(mt);
                nums[t][i] = n;
                sets[t].insert(n);
            }
            // unique set ?
            if (sets[t].size() < size_t(f * t)) {
                sort(begin(nums[t]), end(nums[t]));
                nums[t].erase(unique(begin(nums[t]), end(nums[t])), end(nums[t]));
            }
            shuffle(begin(nums[t]), end(nums[t]), mt);
        }

        START(test);
        for (int t = 0; t < T; t++) {
            int len = nums[t].size();
            for (int i = 0; i < len; i++) {
                sets[t].erase(sets[t].find(nums[t][i]));
            }
        }
        TIME(test);

        print(" {:>12} {:6}ms\n", name, TIME_MS(test));
    }
};

struct minmax_test {
    template <typename Set>
    static void run(const string& name, int M, int T, int f, int q) {
        intd distn(0, M);

        START(test);
        size_t count = 0;
        long sum = 0;
        for (int t = 0; t < T; t++) {
            Set set;
            for (int i = 0; i < t; i++) {
                for (int j = 0; j < f; j++) {
                    set.insert(distn(mt));
                }
                for (int j = 0; j < q; j++) {
                    count += distn(mt) < *set.begin();
                }
                for (int j = 0; j < q; j++) {
                    count += *set.rbegin() < distn(mt);
                }
                sum += accumulate(begin(set), end(set), 0L);
            }
        }
        TIME(test);

        print(" {:>12} {:6}ms  (count/sum: {}, {})\n", name, TIME_MS(test), count, sum);
    }
};

} // namespace bstree_performance_testers

template <typename Fn, typename... Args>
void run(Args&&... args) {
    Fn::template run<set<int>>("set"s, args...);
    Fn::template run<bs_set<int>>("bs_set"s, args...);
    Fn::template run<multiset<int>>("multiset"s, args...);
    Fn::template run<bs_multiset<int>>("bs_multiset"s, args...);
}

int main() {
    print("# minmax compare and accumulate (int) low inserts -----\n");
    RUN_BLOCK(run<minmax_test>(1'000'000, 1000, 2, 50));

    print("# minmax compare and accumulate (int) many inserts -----\n");
    RUN_BLOCK(run<minmax_test>(1'000'000, 500, 10, 20));

    print("# ordered hinted insertion begin (int) few collisions -----\n");
    RUN_BLOCK(run<ordered_insert_hint_begin_test>(1'000'000, 2000, 20));

    print("# ordered hinted insertion begin (int) many collisions -----\n");
    RUN_BLOCK(run<ordered_insert_hint_begin_test>(1'000, 2000, 20));

    print("# ordered hinted insertion end (int) few collisions -----\n");
    RUN_BLOCK(run<ordered_insert_hint_end_test>(1'000'000, 2000, 20));

    print("# ordered hinted insertion end (int) many collisions -----\n");
    RUN_BLOCK(run<ordered_insert_hint_end_test>(1'000, 2000, 20));

    print("# unordered insertion (int) few collisions -----\n");
    RUN_BLOCK(run<unordered_insert_test>(1'000'000, 1000, 20));

    print("# unordered insertion (int) moderate collisions -----\n");
    RUN_BLOCK(run<unordered_insert_test>(25'000, 1000, 20));

    print("# unordered insertion (int) many collisions -----\n");
    RUN_BLOCK(run<unordered_insert_test>(1'000, 1000, 20));

    print("# count query (int) unlikely -----\n");
    RUN_BLOCK(run<count_query_test>(1'000'000, 500, 20, 100));

    print("# count query (int) moderate -----\n");
    RUN_BLOCK(run<count_query_test>(25'000, 500, 20, 100));

    print("# count query (int) likely -----\n");
    RUN_BLOCK(run<count_query_test>(1'000, 500, 20, 100));

    print("# random erase test (int) unlikely -----\n");
    RUN_BLOCK(run<random_erase_test>(1'000'000, 500, 20, 100));

    print("# random erase test (int) moderate -----\n");
    RUN_BLOCK(run<random_erase_test>(25'000, 500, 20, 100));

    print("# random erase test (int) likely -----\n");
    RUN_BLOCK(run<random_erase_test>(1'000, 500, 20, 100));

    print("# perfect unordered erase test (int) -----\n");
    RUN_BLOCK(run<perfect_unordered_erase_test>(1'000'000, 1000, 20));

    return 0;
}
