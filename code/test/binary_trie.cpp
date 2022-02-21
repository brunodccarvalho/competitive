#include "test_utils.hpp"
#include "struct/binary_trie.hpp"

void stress_test_xor_range() {
    constexpr int B = 9, MAX = 1 << B;
    binary_trie<int, B> trie;
    multiset<int> nums;

    uniform_int_distribution<int> anynum(0, MAX - 1);

    for (int n = 0; n < MAX; n++) {
        int cnt = rand_unif<int>(0, 4);
        trie.insert(n, cnt, cnt);
        for (int c = 0; c < cnt; c++) {
            nums.insert(n);
        }
    }

    LOOP_FOR_DURATION_TRACKED_RUNS (5s, now, runs) {
        print_time(now, 5s, "stress xor range ({} runs)", runs);

        int x = anynum(mt);
        auto [l, r] = different<int>(0, MAX);
        int got = trie.count_xor_range(x, l, r);
        int exp = 0;
        for (int n : nums) {
            exp += l <= (n ^ x) && (n ^ x) < r;
        }
        assert(got == exp);
        assert(int(nums.count(x)) == trie.contains(x));
    }
}

int main() {
    RUN_BLOCK(stress_test_xor_range());
    return 0;
}
