#pragma once

#include "lib/test_progress.hpp"
#include "lib/test_chrono.hpp"

struct event_time_tracker {
    discrete_distribution<int> eventd;
    chrono::steady_clock::time_point start_timepoint;
    vector<chrono::nanoseconds> event_time_elapsed;
    vector<long> event_frequency;
    int latest, N;

    event_time_tracker(initializer_list<int> arr) : eventd(begin(arr), end(arr)) {
        N = eventd.probabilities().size();
        event_time_elapsed.resize(N), event_frequency.resize(N);
    }
    template <typename Array>
    explicit event_time_tracker(const Array& arr) : eventd(begin(arr), end(arr)) {
        N = eventd.probabilities().size();
        event_time_elapsed.resize(N), event_frequency.resize(N);
    }

    void set_event(int event) { latest = event; }

    int next_event() { return latest = eventd(mt); }

    void start_event(int event) { set_event(event), start(); }

    void start() { start_timepoint = chrono::steady_clock::now(); }

    void time() {
        auto duration = chrono::steady_clock::now() - start_timepoint;
        event_frequency[latest]++;
        event_time_elapsed[latest] += duration;
    }

    template <typename Names, typename Key = int>
    void pretty_log(Names namesmap) const {
        for (int i = 0; i < N; i++) {
            if (event_frequency[i] > 0) {
                string name = namesmap[Key(i)];
                long frequency = event_frequency[i];
                double total_ns = event_time_elapsed[i].count();
                double total_ms = total_ns / 1e6;
                double each_ns = frequency ? (total_ns / frequency) : 0;
                double each_1000ms = each_ns / 1e3;
                printcl("{:15} x{:<8} {:8.2f}ms {:9.2f}ms/1000\n", //
                        name, frequency, total_ms, each_1000ms);
            }
        }
    }
};
