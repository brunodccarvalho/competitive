#pragma once

#include "numeric/bigint.hpp"
#include "random.hpp"

string random_numeric_string(int digits, int base = 10, double neg_p = 0.0,
                             bool zero = false) {
    static boold signd(0.3);
    boold negd(neg_p);

    string s = rand_string(digits, '0', '0' + base - 1);
    auto i = s.find_first_not_of('0');
    if (i == string::npos)
        return zero ? "0" : "1";
    s = s.substr(i);
    if (negd(mt))
        return "-" + s;
    else if (signd(mt))
        return "+" + s;
    else
        return s;
}

bigint random_bigint(int digits, int base = 10, double neg_p = 0.0, bool zero = false) {
    return bigint(random_numeric_string(digits, base, neg_p, zero), base);
}

bigint bigpow(int n, int base = 10) { return bigint("1" + string(n, '0'), base); }
