#include "test_utils.hpp"
#include "strings/boyer_moore.hpp"
#include "strings/kmp.hpp"
#include "strings/z_search.hpp"
#include "lib/strings.hpp"

auto naive_search_all(const string& haystack, const string& needle) {
    int P = needle.size(), T = haystack.size();
    vector<int> indices;
    const char* np = needle.data();
    const char* hp = haystack.data();
    for (int i = 0; i + P <= T; i++) {
        if (equal(np, np + P, hp + i))
            indices.push_back(i);
    }
    return indices;
}

void stress_test_string_searchers() {
    vector<string> needles, haystacks;

    auto add = [&](vector<string>& strs, vector<string>&& more) {
        strs.insert(end(strs), begin(more), end(more));
    };

    add(needles, generate_all_strings(1, 6, 'a', 'c'));
    add(needles, {"abab", "ababa", "ababab", "abababab", "abba", "abbaab", "abbaabba"});
    add(needles, {"cabab", "abcabababc", "ababcabcab", "ababcab", "abcabab"});
    add(needles, {"aabaa", "aaaab", "baaaa", "ababa", "bcabcaabc"});
    add(needles, {"accbcc", "accbccacbc", "accbccaccbcc", "acbcacbc"});
    add(needles, {".", ",!?#:", "aa.bb", "a,", ",a"});
    add(needles, rand_strings(37, 7, 15, 'a', 'c'));
    add(needles, rand_strings(20, 16, 40, 'a', 'c'));

    add(haystacks, {"...", "#?!", "?!", "!?", "   ", "abcdabc", ",bab,", ",aba,"});
    add(haystacks, generate_all_strings(0, 7, 'a', 'c'));
    add(haystacks, rand_strings(3'000, 8, 15, 'a', 'c'));
    add(haystacks, rand_strings(1'000, 16, 70, 'a', 'c'));
    add(haystacks, rand_strings(100, 71, 1000, 'a', 'c'));
    add(haystacks, rand_strings(20, 1001, 10000, 'a', 'c'));

    int N = needles.size(), H = haystacks.size();
    print(" no. needles: {}\n", N);
    print(" no. haystacks: {}\n", H);

    for (int i = 0; i < N; i++) {
        print_progress(i, N, "stress test string searchers");
        auto needle = needles[i];
        KMP kmp(needle);
        BoyerMoore bm(needle);
        for (auto haystack : haystacks) {
            auto i1 = naive_search_all(haystack, needle);
            auto i2 = kmp_search_all(haystack, kmp);
            auto i3 = boyer_moore_search_all(haystack, bm);
            auto i4 = z_search_all(haystack, needle);
            assert(i1 == i2);
            assert(i1 == i3);
            assert(i1 == i4);
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_string_searchers());
    return 0;
}
