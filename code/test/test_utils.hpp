#pragma once

#include "random.hpp"
#include "lib/test_chrono.hpp"
#include "lib/test_progress.hpp"

template <typename Container>
bool all_eq(const Container& v) {
    return v.empty() || equal(next(begin(v)), end(v), begin(v));
}

template <typename U, typename V, typename String>
void print_time_table(const map<pair<U, V>, String>& times, const string& label) {
    printcl("===== {}\n{}=====\n", label, format_pair_map(times));
}

template <typename U, typename V, typename W, typename String>
void print_time_table(const map<tuple<U, V, W>, String>& times, const string& label) {
    printcl("===== {}\n{}=====\n", label, format_tuple_map(times, true));
}

template <typename U>
void print_time_table(const vector<vector<U>>& times, const string& label) {
    printcl("===== {}\n{}=====\n", label, mat_to_string(times));
}

#define RUN_BLOCK(test)                                                  \
    do {                                                                 \
        printcl("{:<10} === {}\n", "RUN", #test);                        \
        START(runner);                                                   \
        test;                                                            \
        TIME(runner);                                                    \
        printcl("OK {:>6.2f}s === {}\n", 1e-3 * TIME_MS(runner), #test); \
    } while (0)

#define RUN_SHORT(test)                                                  \
    do {                                                                 \
        START(runner);                                                   \
        test;                                                            \
        TIME(runner);                                                    \
        printcl("OK {:>6.2f}s === {}\n", 1e-3 * TIME_MS(runner), #test); \
    } while (0)
