#include "test_utils.hpp"
#include "struct/segtree.hpp"
#include "struct/segtree_nodes.hpp"

auto after_searcher(int t) {
    return [t](simple_segnode v) { return v.value >= t; };
}
auto strict_after_searcher(int t) {
    return [t](simple_segnode v) { return v.value > t; };
}
auto before_searcher(int t) {
    return [t](simple_segnode v) { return v.value <= t; };
}
auto strict_before_searcher(int t) {
    return [t](simple_segnode v) { return v.value < t; };
}

void stress_test_segtree_binary_search() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (20s, now, 10000, runs) {
        print_time(now, 20s, "stress segtree binary search ({} runs)", runs);

        int N = rand_unif<int>(1, 200);
        auto arr = rands_unif<int>(N, 1, 10000);

        segtree<simple_segnode> seg(N, arr);

        // Prefix search whole segtree
        {
            auto pref = arr;
            for (int i = 1; i < N; i++)
                pref[i] += pref[i - 1];

            for (int i = 0; i < N; i++) {
                auto sum = pref[i];

                auto lower = after_searcher(sum);
                auto upper = strict_after_searcher(sum);

                assert(seg.prefix_binary_search(lower).first == i);
                assert(seg.prefix_binary_search(upper).first == i + 1);
            }
        }

        // Suffix search whole segtree
        {
            auto suff = arr;
            for (int i = N - 2; i >= 0; i--)
                suff[i] += suff[i + 1];

            for (int i = 0; i < N; i++) {
                auto sum = suff[i];

                auto lower = before_searcher(sum);
                auto upper = strict_before_searcher(sum);

                assert(seg.suffix_binary_search(lower).first == i);
                assert(seg.suffix_binary_search(upper).first == i + 1);
            }
        }

        // Now the hard part, binary search on subsegments
        for (int l = 0; l < N; l++) {
            for (int r = l + 1; r <= N; r++) {
                int S = r - l;

                vector<int> pref(begin(arr) + l, begin(arr) + r);
                for (int i = 1; i < S; i++)
                    pref[i] += pref[i - 1];

                for (int i = l; i < r; i++) {
                    auto sum = pref[i - l];

                    auto lower = after_searcher(sum);
                    auto upper = strict_after_searcher(sum);

                    assert(seg.prefix_range_search(l, r, lower).first == i);
                    assert(seg.prefix_range_search(l, r, upper).first == i + 1);
                }

                vector<int> suff(begin(arr) + l, begin(arr) + r);
                for (int i = S - 2; i >= 0; i--)
                    suff[i] += suff[i + 1];

                for (int i = l; i < r; i++) {
                    auto sum = suff[i - l];

                    auto lower = before_searcher(sum);
                    auto upper = strict_before_searcher(sum);

                    assert(seg.suffix_range_search(l, r, lower).first == i);
                    assert(seg.suffix_range_search(l, r, upper).first == i + 1);
                }
            }
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_segtree_binary_search());
    return 0;
}
