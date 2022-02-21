#include "test_utils.hpp"
#include "strings/strings.hpp"

void unit_test_good_suffix() {
    string ss[] = {
        "addbddcdd",      // 1 2 3 4 5 6 7 8 9 10
                          // a d d b d d c d d
                          // . . . . . . 3 1 2
        "ABBABAB",        // A B B A B A B
                          // 5 6 4 5 6 7 7 8
                          // . . . 2 . 4 1
        "ccacc",          // c c a c c
                          // . . . 1 2
        "CABAB",          // C A B A B
                          // 5 3 4 5 5 6
                          // . . 2 . 1
        "ABCABBACCABCAB", // A B C A B B A C C A B C A B
                          // x x x x x x x x x y 3 y 8 1, x=9, y=12
    };
    for (const string& s : ss) {
        print("    border [{}]: {}\n", s, build_border_function(s));
        print(" true/good [{}]: {}\n", s, build_good_suffix_border_table(s, true).first);
        print("false/good [{}]: {}\n", s, build_good_suffix_border_table(s, false).first);
    }
}

void unit_test_manachers() {
    string ss[] = {
        "abcdfdcecba"s,
        "abababc"s,
        "abaabbbaaaacca"s,
    };
    for (string& s : ss) {
        cout << build_manachers(s) << endl;
    }
}

void unit_test_prefix_function() {
    string ss[] = {
        "abcabcd"s, // 0,0,0,1,2,3,0
        "aabaaab"s, // 0,1,0,1,2,2,3
    };
    for (string& s : ss) {
        cout << build_prefix_function(s) << endl;
    }
}

void unit_test_z_function() {
    string ss[] = {
        "aaaaa"s,   // 0,4,3,2,1
        "aaabaab"s, // 0,2,1,0,2,1,0
        "abacaba"s, // 0,0,1,0,3,0,1
    };
    for (string& s : ss) {
        cout << build_z_function(s) << endl;
    }
}

void unit_test_lyndon() {
    string ss[] = {
        "efdcba"s,         // ef d c b a | 0,2,3,4,5
        "abaabaaabaaaab"s, // ab aab aaab aaaab | 0,2,5,9
        "abacaba"s,        // abac ab a | 0,4,6
    };
    for (string& s : ss) {
        cout << build_lyndon_factorization(s) << endl;
    }
}

void unit_test_suffix_array() {
    using vi = vector<int>;
    string s;
    vi sa, lcp;

    s = "abbaacaaabab"s;
    sa = build_cyclic_shifts(s);
    lcp = build_lcp_array(s, sa);

    for (int i = 0, N = s.size(); i < N; i++) {
        print("{:2}: {}\n", i, s.substr(sa[i]) + s.substr(0, sa[i]));
    }
    print(" sa: {}\nlcp: {}\n", sa, lcp);
}

int main() {
    RUN_SHORT(unit_test_manachers());
    RUN_SHORT(unit_test_prefix_function());
    RUN_SHORT(unit_test_z_function());
    RUN_SHORT(unit_test_lyndon());
    RUN_SHORT(unit_test_good_suffix());
    RUN_SHORT(unit_test_suffix_array());
    return 0;
}
