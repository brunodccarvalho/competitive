#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename D>
struct kahan {
    using value_type = D;
    D sum = 0, c = 0;

    explicit kahan(D value = 0, D comp = 0) : sum(value), c(comp) {}

    operator D() const noexcept { return sum; }

    friend bool operator==(kahan a, kahan b) { return a.sum == b.sum; }
    friend bool operator!=(kahan a, kahan b) { return a.sum != b.sum; }
    friend bool operator<(kahan a, kahan b) { return a.sum < b.sum; }
    friend bool operator>(kahan a, kahan b) { return a.sum > b.sum; }
    friend bool operator<=(kahan a, kahan b) { return a.sum <= b.sum; }
    friend bool operator>=(kahan a, kahan b) { return a.sum >= b.sum; }

    kahan& set(D x) { return sum = x, c = 0, *this; }

    friend kahan& operator+=(kahan& k, kahan add) {
        D y = add.sum + k.c - add.c, t = k.sum + y;
        k.c = (t - k.sum) - y, k.sum = t;
        return k;
    }
    friend kahan& operator-=(kahan& k, kahan sub) {
        D y = sub.sum - k.c - sub.c, t = k.sum - y;
        k.c = (t - k.sum) + y, k.sum = t;
        return k;
    }
    friend kahan& operator+=(kahan& k, D add) {
        D y = add + k.c, t = k.sum + y;
        k.c = (t - k.sum) - y, k.sum = t;
        return k;
    }
    friend kahan& operator-=(kahan& k, D sub) {
        D y = sub - k.c, t = k.sum - y;
        k.c = (t - k.sum) + y, k.sum = t;
        return k;
    }
    friend kahan& operator*=(kahan& k, D mul) { return k.sum *= mul, k.c *= mul, k; }
    friend kahan& operator/=(kahan& k, D div) { return k.sum /= div, k.c /= div, k; }
    friend kahan operator+(kahan k, D add) { return k += add; }
    friend kahan operator-(kahan k, D sub) { return k -= sub; }
    friend kahan operator*(kahan k, D mul) { return k *= mul; }
    friend kahan operator/(kahan k, D div) { return k /= div; }
    friend kahan operator+(kahan k) { return kahan(+k.sum, +k.c); }
    friend kahan operator-(kahan k) { return kahan(-k.sum, -k.c); }

    friend string to_string(kahan k) { return to_string(k.sum); }
    friend ostream& operator<<(ostream& out, kahan k) { return out << to_string(k.sum); }
    friend istream& operator>>(istream& in, kahan& k) { return in >> k.sum; }
};
