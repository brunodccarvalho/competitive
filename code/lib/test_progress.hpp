#pragma once

#include <unistd.h>
#include "lib/test_chrono.hpp"
#include "formatting.hpp"

bool cout_is_terminal() {
    static int is = -1;
    return is < 0 ? (is = isatty(STDOUT_FILENO)) : is;
}

auto& log_destination() { return cout_is_terminal() ? std::cout : std::cerr; }

void clear_line() { cout_is_terminal() ? fmt::print("\r\033[2K") : (void)fflush(stdout); }

template <typename... Ts>
void printcl(Ts&&... args) {
    auto& dest = log_destination();
    fmt::print(dest, "\r\033[2K");
    fmt::print(dest, forward<Ts>(args)...);
    flush(dest);
}

template <typename... Ts>
void putcln(Ts&&... args) {
    auto& dest = log_destination();
    fmt::print(dest, "\r\003[2K");
    putln(dest, forward<Ts>(args)...);
    flush(dest);
}

void print_progress(long i, long N) {
    double percent = 100.0 * (i + 1) / N;
    int digits = to_string(N).size();
    printcl("{:5.1f}% {:>{}}/{}", percent, i + 1, digits, N);
}

template <typename T>
void print_progress(long i, long N, T&& content) {
    double percent = 100.0 * (i + 1) / N;
    int digits = to_string(N).size();
    string txt = fmt::format("{}", forward<T>(content));
    printcl("{:5.1f}% {:>{}}/{} {}", percent, i + 1, digits, N, txt);
}

template <typename... Ts>
void print_progress(long i, long N, string_view fmt, Ts&&... args) {
    return print_progress(i, N, fmt::format(fmt, forward<Ts>(args)...));
}

template <typename... Ts>
void print_regular(long i, long N, long step, Ts&&... args) {
    if ((i == 0 || (i + 1) % step == 0)) {
        print_progress(i, N, forward<Ts>(args)...);
    }
}

template <typename T1, typename T2, typename T>
void print_time_view(T1 now, T2 duration, T&& content) {
    double percent = 100.0 * now / duration;
    auto txt = fmt::format("{}", forward<T>(content));
    printcl("{:5.1f}% {}", percent, txt);
}

template <typename T1, typename T2, typename... Ts>
void print_time_view(T1 now, T2 duration, string_view fmt, Ts&&... args) {
    double percent = 100.0 * now / duration;
    auto txt = fmt::format(fmt, forward<Ts>(args)...);
    printcl("{:5.1f}% {}", percent, txt);
}

template <typename T1, typename T2, typename... Ts>
void print_time(T1 now, T2 duration, Ts&&... args) {
    static const chrono::milliseconds step = 0ms;
    static chrono::nanoseconds next_now = 30ns;
    if ((sizeof...(Ts) > 1 || now == 0ns || now >= next_now)) {
        next_now = now == 0ns ? step : now + step;
        print_time_view(now, duration, forward<Ts>(args)...);
    }
}

template <typename... Ts>
[[noreturn]] void fail(Ts&&... args) {
    fmt::print(log_destination(), "\n"), clear_line();
    fmt::print(log_destination(), "Error: {}", fmt::format(forward<Ts>(args)...));
    exit(1);
}
