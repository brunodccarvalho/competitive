#pragma once

#include <bits/stdc++.h>
using namespace std;

using ns = chrono::nanoseconds;
using us = chrono::microseconds;
using ms = chrono::milliseconds;

#define CONCAT(x, y) x##y

#define LOOP_FOR_DURATION_IMPL(z, duration)                   \
    auto CONCAT(loop_start, z) = chrono::steady_clock::now(); \
    while (chrono::steady_clock::now() - CONCAT(loop_start, z) < duration)

#define LOOP_FOR_DURATION_OR_RUNS_IMPL(z, duration, max_runs)            \
    auto CONCAT(loop_start, z) = chrono::steady_clock::now();            \
    for (auto loop_counter_##z = 0;                                      \
         loop_counter_##z < max_runs &&                                  \
         chrono::steady_clock::now() - CONCAT(loop_start, z) < duration; \
         loop_counter_##z++)

#define LOOP_FOR_DURATION_TRACKED_IMPL(z, duration, now)                              \
    auto CONCAT(loop_start, z) = chrono::steady_clock::now();                         \
    for (auto now = decltype(chrono::steady_clock::now() - CONCAT(loop_start, z))(0); \
         now < duration; now = chrono::steady_clock::now() - CONCAT(loop_start, z))

#define LOOP_FOR_DURATION_TRACKED_RUNS_IMPL(z, duration, now, runs)                   \
    auto runs = 0;                                                                    \
    auto CONCAT(loop_start, z) = chrono::steady_clock::now();                         \
    for (auto now = decltype(chrono::steady_clock::now() - CONCAT(loop_start, z))(0); \
         now < duration;                                                              \
         now = chrono::steady_clock::now() - CONCAT(loop_start, z), runs++)

#define LOOP_FOR_DURATION_OR_RUNS_TRACKED_IMPL(z, duration, now, max_runs, runs)      \
    auto runs = 0;                                                                    \
    auto CONCAT(loop_start, z) = chrono::steady_clock::now();                         \
    for (auto now = decltype(chrono::steady_clock::now() - CONCAT(loop_start, z))(0); \
         runs < max_runs && now < duration;                                           \
         now = chrono::steady_clock::now() - CONCAT(loop_start, z), runs++)

#define LOOP_FOR_DURATION(in_duration) LOOP_FOR_DURATION_IMPL(__COUNTER__, in_duration)

#define LOOP_FOR_DURATION_OR_RUNS(in_duration, in_max_runs) \
    LOOP_FOR_DURATION_OR_RUNS_IMPL(__COUNTER__, in_duration, in_max_runs)

#define LOOP_FOR_DURATION_TRACKED(in_duration, out_now) \
    LOOP_FOR_DURATION_TRACKED_IMPL(__COUNTER__, in_duration, out_now)

#define LOOP_FOR_DURATION_TRACKED_RUNS(in_duration, out_now, out_runs) \
    LOOP_FOR_DURATION_TRACKED_RUNS_IMPL(__COUNTER__, in_duration, out_now, out_runs)

#define LOOP_FOR_DURATION_OR_RUNS_TRACKED(in_duration, out_now, in_max_runs, out_runs) \
    LOOP_FOR_DURATION_OR_RUNS_TRACKED_IMPL(__COUNTER__, in_duration, out_now,          \
                                           in_max_runs, out_runs)
