#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Aho-Corasick string automaton
 * Edit A and chash before using. Add extra preprocessing in the constructor.
 * Requires 4(A+c)S bytes where S=#states and S<=W=#dict length. Not adequate if A>50.
 *
 * Complexity: O(AS+M) construction, O(AS) space, O(N) for main queries.
 * Reference: https://github.com/indy256/codelibrary
 */
template <typename T = char>
struct aho_corasick {
    static constexpr int A = 26;
    static constexpr int chash(T value) { return value - 'a'; }

    struct Node {
        int next[A] = {};
        int parent = 0, len = 0, ch = 0;
        int escape = 0, link = 0; // nearest leaf / suffix link
        int nmatches = 0, wordid = -1;
        Node() = default;
        Node(int parent, int len, int ch) : parent(parent), len(len), ch(ch) {}
    };

    vector<Node> node;

    aho_corasick() = default;

    template <typename Vec>
    explicit aho_corasick(const vector<Vec>& words) : node(1) {
        for (int i = 0, W = words.size(); i < W; i++) {
            assert(!words[i].empty());
            int v = 0, len = 1;
            for (auto value : words[i]) {
                int c = chash(value);
                if (!node[v].next[c]) {
                    node[v].next[c] = node.size();
                    node.emplace_back(v, len, c);
                }
                v = node[v].next[c], len++;
            }
            node[v].escape = v;

            // Preprocess: ignore repeated words
            node[v].nmatches = 1;
            node[v].wordid = i;
        }

        vector<int> bfs(num_nodes());
        int s = 0, t = 1;
        while (s < t) {
            int v = bfs[s++], u = node[v].link;
            if (node[v].escape == 0) {
                node[v].escape = node[u].escape;
            }
            for (int c = 0; c < A; c++) {
                int& w = node[v].next[c];
                if (w) {
                    bfs[t++] = w;
                    node[w].link = v ? node[u].next[c] : 0;
                } else {
                    w = node[u].next[c];
                }
            }
            // Preprocess:
            node[v].nmatches += node[u].nmatches;
        }
        assert(t == num_nodes());
    }

    int num_nodes() const { return node.size(); }

    // Count number of distinct indices where words end.
    template <typename Vec>
    int count_unique_matches(const Vec& text) const {
        int matches = 0;
        for (int v = 0, i = 0, N = text.size(); i < N; i++) {
            v = node[v].next[chash(text[i])];
            matches += node[v].escape > 0;
        }
        return matches;
    }

    // Count total number of matches across all words and indices
    template <typename Vec>
    long count_matches(const Vec& text) const {
        long matches = 0;
        for (int v = 0, i = 0, N = text.size(); i < N; i++) {
            v = node[v].next[chash(text[i])];
            matches += node[v].nmatches;
        }
        return matches;
    }

    // For each index i, find the longest dictionary word ending at text[i] (inclusive)
    template <typename Vec>
    vector<int> longest_each_index(const Vec& text) const {
        vector<int> longest(text.size(), -1);
        for (int v = 0, i = 0, N = text.size(); i < N; i++) {
            v = node[v].next[chash(text[i])];
            longest[i] = node[node[v].escape].wordid;
        }
        return longest;
    }

    // Call fn(i, wordid) for every match <i, longest wordid>
    template <typename Fn, typename Vec>
    void visit_longest_each_index(const Vec& text, Fn&& fn, bool skipbad = true) const {
        for (int v = 0, i = 0, N = text.size(); i < N; i++) {
            v = node[v].next[chash(text[i])];
            int w = node[node[v].escape].wordid;
            if (w != -1 || !skipbad)
                fn(i, w);
        }
    }

    // Call fn(i, wordid) for every match <i, wordid>
    template <typename Fn, typename Vec>
    void visit_all(const Vec& text, Fn&& fn) const {
        for (int v = 0, i = 0, N = text.size(); i < N; i++) {
            v = node[v].next[chash(text[i])];
            int u = node[v].escape;
            while (u != 0)
                fn(i, node[u].wordid), u = node[node[u].link].escape;
        }
    }

    // Find first occurrence of any dictionary word in text. Returns <i, longest wordid>
    template <typename Vec>
    pair<int, int> find_first(const Vec& text) const {
        for (int v = 0, i = 0, N = text.size(); i < N; i++) {
            v = node[v].next[chash(text[i])];
            int w = node[node[v].escape].wordid;
            if (w != 0)
                return {i, w};
        }
        return {-1, -1};
    }
};
