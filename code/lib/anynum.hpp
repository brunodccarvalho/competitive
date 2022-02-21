#pragma once

#include "numeric/frac.hpp"
#include "random.hpp"

template <typename T, typename O = T>
O uniform_gen(long minv, long maxv, long maxd = 2) {
    assert(max(abs(minv), abs(maxv)) <= LONG_MAX / maxd);
    if constexpr (is_same_v<T, frac<T>>) {
        long d = longd(1, maxd)(mt);
        longd dist(minv * d, maxv * d);
        return frac(dist(mt), d);
    } else if constexpr (is_integral_v<T>) {
        return longd(minv, maxv)(mt);
    } else if constexpr (is_arithmetic_v<T>) {
        long n = longd(minv * maxd, maxv * maxd)(mt);
        return T(n) / T(maxd);
    }
}

template <typename T, typename O = T>
auto uniform_gen_many(int n, long minv, long maxv, long maxd = 2) {
    vector<O> nums(n);
    for (int i = 0; i < n; i++) {
        nums[i] = uniform_gen<T, O>(minv, maxv, maxd);
    }
    return nums;
}
