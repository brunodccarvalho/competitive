#pragma once

#include "lib/test_progress.hpp"
#include "numeric/chrono.hpp"

string format_duration(double duration) {
    constexpr int64_t THRESHOLD = 1000;
    if (duration <= 0) {
        return "0ms";
    } else if (duration <= THRESHOLD) {
        return fmt::format("{:.1f}ns", duration);
    } else if (duration <= 1'000 * THRESHOLD) {
        return fmt::format("{:.2f}us", duration / 1e3);
    } else if (duration <= 1'000'000 * THRESHOLD) {
        return fmt::format("{:.2f}ms", duration / 1e6);
    } else {
        return fmt::format("{:.2f}s", duration / 1e9);
    }
}

string format_duration(chrono::nanoseconds duration) {
    return format_duration(duration.count());
}

void print_duration(chrono::nanoseconds duration, const string& label) {
    printcl(" {:>12} -- {}\n", format_duration(duration), label);
}

void print_each_duration(double duration, const string& label) {
    printcl(" {:>12} each -- {}\n", format_duration(duration), label);
}

/**
 * CHRONO MACROS
 * To time a portion of code once, do:
 *     START($var);
 *     run_code();
 *     TIME($var);
 *     PRINT_TIME($var);
 *
 * To accumulate a portion of a loop, do:
 *     START_ACC($var);
 *     for (...) {
 *         ...
 *         START($var);
 *         run_code();
 *         ADD_TIME($var);
 *         ...
 *     }
 *     PRINT_TIME($var);
 *
 * To print a time matrix (works with 3 dims as well):
 *     map<pair<U, V>, string> times;
 *     for (...) {
 *         ...
 *         times.insert({{u, v}, FORMAT_EACH(var, runs)});
 *     }
 *     print_time_table(times);
 */
#define START_ACC(var) chrono::nanoseconds time_##var = 0ns
#define START(var)     auto now_##var = chrono::steady_clock::now()
#define ADD_TIME(var)  time_##var += CUR_TIME(var)
#define CUR_TIME(var)  (chrono::steady_clock::now() - now_##var)
#define TIME(var)      chrono::nanoseconds time_##var = CUR_TIME(var)

#define TIME_MS(var) chrono::duration_cast<chrono::milliseconds>(time_##var).count()
#define TIME_US(var) chrono::duration_cast<chrono::microseconds>(time_##var).count()
#define TIME_NS(var) chrono::duration_cast<chrono::nanoseconds>(time_##var).count()

#define EACH_MS(var, runs) (1e-6 * TIME_NS(var) / (runs))
#define EACH_US(var, runs) (1e-3 * TIME_NS(var) / (runs))
#define EACH_NS(var, runs) (1.0 * TIME_NS(var) / (runs))

#define RATIO(var1, var2) (TIME_NS(var2) > 0 ? 1.0 * TIME_NS(var1) / TIME_NS(var2) : 0.0)

#define TIME_BLOCK(var)                                                             \
    for (auto [_track##var, now_##var] = make_pair(1, chrono::steady_clock::now()); \
         _track##var--; print_duration(CUR_TIME(var), #var))

#define ADD_TIME_BLOCK(var)                                                         \
    for (auto [_track##var, now_##var] = make_pair(1, chrono::steady_clock::now()); \
         _track##var--; ADD_TIME(var))

#define PRINT_TIME(var)  print_duration(time_##var, #var)
#define FORMAT_TIME(var) (TIME_NS(var) > 0 ? format_duration(TIME_NS(var)) : ""s)

#define PRINT_EACH(var, runs) print_each_duration(EACH_NS(var, runs), #var)
#define FORMAT_EACH(var, runs) \
    (TIME_NS(var) > 0 ? format_duration(EACH_NS(var, runs)) : ""s)

#define PRINT_RATIO(var1, var2)                 \
    if (TIME_NS(var1) > 0 && TIME_NS(var2) > 0) \
    printcl(" {:.2f} speedup -- {} / {}", RATIO(var1, var2), #var1, #var2)

#define FORMAT_RATIO(var1, var2) \
    (TIME_NS(var1) > 0 && TIME_NS(var2) > 0 ? format("{:.2f}", RATIO(var1, var2)) : ""s)

#define START_ACC2(var1, var2) \
    START_ACC(var1);           \
    START_ACC(var2)
#define START_ACC3(var1, var2, var3) \
    START_ACC(var1);                 \
    START_ACC(var2);                 \
    START_ACC(var3)
#define START_ACC4(var1, var2, var3, var4) \
    START_ACC(var1);                       \
    START_ACC(var2);                       \
    START_ACC(var3);                       \
    START_ACC(var4)
#define START_ACC5(var1, var2, var3, var4, var5) \
    START_ACC(var1);                             \
    START_ACC(var2);                             \
    START_ACC(var3);                             \
    START_ACC(var4);                             \
    START_ACC(var5)

#define PRINT_TIME2(var1, var2) \
    PRINT_TIME(var1);           \
    PRINT_TIME(var2)
#define PRINT_TIME3(var1, var2, var3) \
    PRINT_TIME(var1);                 \
    PRINT_TIME(var2);                 \
    PRINT_TIME(var3)
#define PRINT_TIME4(var1, var2, var3, var4) \
    PRINT_TIME(var1);                       \
    PRINT_TIME(var2);                       \
    PRINT_TIME(var3);                       \
    PRINT_TIME(var4)
#define PRINT_TIME5(var1, var2, var3, var4, var5) \
    PRINT_TIME(var1);                             \
    PRINT_TIME(var2);                             \
    PRINT_TIME(var3);                             \
    PRINT_TIME(var4);                             \
    PRINT_TIME(var5)

#define PRINT_EACH2(var1, var2, runs) \
    PRINT_EACH(var1, runs);           \
    PRINT_EACH(var2, runs)
#define PRINT_EACH3(var1, var2, var3, runs) \
    PRINT_EACH(var1, runs);                 \
    PRINT_EACH(var2, runs);                 \
    PRINT_EACH(var3, runs)
#define PRINT_EACH4(var1, var2, var3, var4, runs) \
    PRINT_EACH(var1, runs);                       \
    PRINT_EACH(var2, runs);                       \
    PRINT_EACH(var3, runs);                       \
    PRINT_EACH(var4, runs)
#define PRINT_EACH5(var1, var2, var3, var4, var5, runs) \
    PRINT_EACH(var1, runs);                             \
    PRINT_EACH(var2, runs);                             \
    PRINT_EACH(var3, runs);                             \
    PRINT_EACH(var4, runs);                             \
    PRINT_EACH(var5, runs)
