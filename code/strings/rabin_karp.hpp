#pragma once

#include "hash.hpp"

/**
 * Rabin Karp
 * https://en.wikipedia.org/wiki/Rabin-Karp_algorithm
 */
int rabin_karp_search(const string& text, const string& needle) {
    int T = text.size(), P = needle.size();
    const char *pp = needle.data(), *tp = text.data();
    if (T < P)
        return -1;

    rolling_hasher hash(needle.size());
    size_t ph = hash(pp, pp + P), th = hash(tp, tp + P);

    for (int i = 0; i <= T - P; i++) {
        if (i > 0) {
            th = hash.roll(th, tp[i - 1], tp[i + P - 1]);
        }
        if (ph == th && equal(pp, pp + P, tp + i)) {
            return i;
        }
    }
    return -1;
}

vector<int> rabin_karp_search_all(const string& text, const string& needle) {
    int T = text.size(), P = needle.size();
    const char *pp = needle.data(), *tp = text.data();
    if (T < P)
        return {};

    rolling_hasher hash(needle.size());
    size_t ph = hash(pp, pp + P), th = hash(tp, tp + P);
    vector<int> match;

    for (int i = 0; i <= T - P; i++) {
        if (i > 0) {
            th = hash.roll(th, tp[i - 1], tp[i + P - 1]);
        }
        if (ph == th && equal(pp, pp + P, tp + i)) {
            match.push_back(i);
        }
    }
    return match;
}
