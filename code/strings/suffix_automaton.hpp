#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Suffix DFA string automation
 * Edit A and chash before using. Add extra preprocessing at the end of preprocess().
 * Requires 8(A+c)N bytes, where N is the length of the text. Not adequate if A>50.
 *
 * Complexity: O(AN)  construction (includes preprocess()), O(AN) space
 *
 * Usage is straightforward if the automaton does not need to grow. If it does, call
 * preprocess() after growing the automaton to use the methods that need preprocessing.
 * Usage:
 *     suffix_automaton sa(text);
 *     bool found = sa.contains(word);
 *     long n = sa.count_distinct_substrings();
 *     int m = sa.count_matches(word);
 *     ...
 */
template <typename Vec = string, typename T = typename Vec::value_type>
struct suffix_automaton {
    static constexpr int A = 26;
    static constexpr int chash(T value) { return value - 'a'; }

    struct Node {
        int next[A] = {};
        int len = 0, link = 0; // suffix link
        int ch = 0;
        int numpos = 0;
        bool terminal = false;
        Node() = default;
        Node(int len, int ch) : len(len), ch(ch) {}
    };

    int V, last = 1; // node[0] is empty; last is id of node with entire string
    vector<Node> node;
    vector<int> pi;

    suffix_automaton() : V(2), node(2) {}
    explicit suffix_automaton(const Vec& text) : suffix_automaton() {
        extend(text);
        preprocess();
    }

    void extend(const Vec& s) {
        for (char c : s) {
            extend(c);
        }
    }

    void extend(T value) {
        int c = chash(value), v = V, p = last;
        node.emplace_back(node[p].len + 1, c), V++;
        while (p && !node[p].next[c]) {
            assert(node[p].len < node[v].len);
            node[p].next[c] = v, p = node[p].link;
        }
        if (p == 0)
            node[v].link = 1;
        else {
            int q = node[p].next[c];
            if (node[p].len + 1 == node[q].len)
                node[v].link = q;
            else {
                int u = node.size();
                node.emplace_back(node[q]), V++;
                node[u].len = node[p].len + 1, node[u].ch = c;
                assert(node[u].len <= node[v].len);
                while (p && node[p].next[c] == q) {
                    node[p].next[c] = u, p = node[p].link;
                }
                node[q].link = node[v].link = u;
            }
        }
        last = v;
    }

    void preprocess() {
        vector<int> cnt(node[last].len + 1), pos(V);
        pi.resize(V);

        for (int v = 0; v < V; v++)
            cnt[node[v].len]++;
        for (int len = 1; len <= node[last].len; len++)
            cnt[len] += cnt[len - 1];
        for (int v = V - 1; v >= 0; v--)
            pos[v] = --cnt[node[v].len];
        for (int v = 0; v < V; v++)
            pi[pos[v]] = v;

        // topological order: pi[0], pi[1], pi[2], ...
        // numpos: number of positions where state v can be found.
        for (int i = V - 1, v = pi[i]; i >= 1; i--, v = pi[i]) {
            node[v].numpos++;
            node[node[v].link].numpos += node[v].numpos;
        }
        node[0].numpos = 0;

        // terminal: whether a state is terminal (corresponds to a suffix)
        int u = last;
        do {
            node[u].terminal = true, u = node[u].link;
        } while (u > 1);
    }

    int get_state(const Vec& word) const {
        int v = 1;
        for (int i = 0, W = word.size(); i < W && v; i++) {
            v = node[v].next[chash(word[i])];
        }
        return v;
    }

    // O(AN) Count the number of distinct substrings (including the empty substring)
    long count_distinct_substrings() const {
        vector<long> dp(V, 1);
        dp[0] = 0;
        for (int i = V - 1; i >= 1; i--) {
            for (int v = pi[i], c = 0; c < A; c++) {
                dp[v] += dp[node[v].next[c]];
            }
        }
        return dp[1];
    }

    // O(W) Does this text contain word
    bool contains(const Vec& word) const { return get_state(word) != 0; }

    // O(W) Length of the longest prefix of word that matches a substring of this text
    int longest_prefix(const Vec& word) const {
        for (int v = 1, i = 0, W = word.size(); i < W; i++) {
            v = node[v].next[chash(word[i])];
            if (v == 0) {
                return i;
            }
        }
        return word.size();
    }

    // O(W) Number of times that word appears in this text
    int count_matches(const Vec& word) const { return node[get_state(word)].numpos; }
};
