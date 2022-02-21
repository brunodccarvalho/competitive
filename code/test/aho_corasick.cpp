#include "test_utils.hpp"
#include "strings/aho_corasick.hpp"

struct visitor {
    int cnt = 0;
    const vector<string>& words;
    explicit visitor(const vector<string>& words) : words(words) {}

    void operator()(int i, int wordid) {
        print("{:3}: index {:2} - {}\n", cnt++, i, words[wordid]);
    }
};

void unit_test_aho_corasick() {
    vector<string> words = {
        "aaab",  "aab",   "aaba",  "aabb",  "abc",  "aca",   "acb", "acba",
        "acbaa", "ba",    "bba",   "bbbbc", "bbbc", "bbbda", "bbc", "bc",
        "caa",   "cadba", "cadbc", "cb",    "cc",   "ccaa",  "ccb",
    };
    string text = "aabababacbcabbaccadbcbacbabcabcababa";
    aho_corasick aho(words);
    print("states: {}\n", aho.num_nodes());
    print("matches: {}\n", aho.count_matches(text));
    aho.visit_all(text, visitor(words));
    print("unique matches: {}\n", aho.count_unique_matches(text));
    aho.visit_longest_each_index(text, visitor(words));
    auto v = aho.longest_each_index(text);
    cout << v << endl;
}

int main() {
    RUN_SHORT(unit_test_aho_corasick());
    return 0;
}
