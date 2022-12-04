#pragma once

#include <bits/stdc++.h>
using namespace std;

auto intpow(int64_t b, int64_t e) {
    int64_t power = 1;
    while (e > 0) {
        if (e & 1) {
            power = power * b;
        }
        e >>= 1;
        b = b * b;
    }
    return power;
}

auto modpow(int64_t base, int64_t e, int64_t mod) {
    int64_t x = 1;
    base = base % mod;
    while (e > 0) {
        if (e & 1)
            x = __int128_t(x) * __int128_t(base) % mod;
        if (e >>= 1)
            base = __int128_t(base) * __int128_t(base) % mod;
    }
    return x;
}

auto modpow(int base, int64_t e, int mod) {
    int x = 1;
    base = base % mod;
    while (e > 0) {
        if (e & 1)
            x = int64_t(x) * int64_t(base) % mod;
        if (e >>= 1)
            base = int64_t(base) * int64_t(base) % mod;
    }
    return x;
}

auto intfac(int n) {
    int64_t f = 1;
    while (n > 1) {
        f = f * n--;
    }
    return f;
}

auto modfac(int n, int64_t m) {
    int64_t f = 1;
    while (n > 1) {
        f = (f * n--) % m;
    }
    return f;
}

auto lcm(int64_t a, int64_t b) { return a * (b / gcd(a, b)); }

auto gcd(int64_t a, int64_t b) {
    while (a != 0) {
        b = b % a;
        swap(a, b);
    }
    return abs(b);
}

auto gcd(int64_t a, int64_t b, int64_t& x, int64_t& y) {
    int64_t xn = 1, yn = 0;
    x = 0, y = 1;
    while (a != 0) {
        int64_t q = b / a;
        b = b % a;
        x = x - q * xn, y = y - q * yn;
        swap(a, b), swap(x, xn), swap(y, yn);
    }
    if (b < 0) {
        b = -b, x = -x, y = -y;
    }
    return b;
}

auto invmod(int64_t a, int64_t m) {
    int64_t x, y, g = gcd(a, m, x, y);
    x = x % m;
    return x < 0 ? x + m : x;
}

/**
 * Compute the smallest exponent x such that a^x = b (mod m)
 * Complexity: O(sqrt(m)), uses square root decomposition
 * Source: kth (https://github.com/kth-competitive-programming/kactl)
 */
int64_t modlog(int64_t a, int64_t b, int64_t m) {
    int64_t n = int64_t(sqrt(m)) + 1, x = 1, f = 1, j = 1;
    unordered_map<int64_t, int64_t> A;
    while (j <= n && (x = f = x * a % m) != b % m)
        A[x * b % m] = j++;
    if (x == b % m)
        return j;
    if (gcd(m, x) == gcd(m, b))
        for (int i = 2; i < n + 2; i++)
            if (A.count(x = x * f % m))
                return n * i - A[x];
    return -1;
}

/**
 * Compute x such that x^2 = a (mod p) where p is prime
 * Complexity: O(log^2 p) heavy
 * Source: kth (https://github.com/kth-competitive-programming/kactl)
 */
int64_t modsqrt(int64_t a, int64_t p) {
    a = a % p, a = a >= 0 ? a : a + p;
    if (a == 0)
        return 0;
    if (modpow(a, (p - 1) / 2, p) != 1)
        return -1; // not a quadratic residue
    if (p % 4 == 3)
        return modpow(a, (p + 1) / 4, p);
    // a^(n+3)/8 or 2^(n+3)/8 * 2^(n-1)/4 works if p % 8 == 5
    int64_t s = p - 1, n = 2;
    int r = 0, m;
    while (s % 2 == 0)
        ++r, s /= 2;
    /// find a non-square mod p
    while (modpow(n, (p - 1) / 2, p) != p - 1)
        ++n;
    int64_t x = modpow(a, (s + 1) / 2, p);
    int64_t b = modpow(a, s, p), g = modpow(n, s, p);
    for (;; r = m) {
        int64_t t = b;
        for (m = 0; m < r && t != 1; ++m)
            t = t * t % p;
        if (m == 0)
            return x;
        int64_t gs = modpow(g, 1L << (r - m - 1), p);
        g = gs * gs % p;
        x = x * gs % p;
        b = b * g % p;
    }
}

// Solve the system a = b[i] (mod p[i]), i = 0,...,n-1
auto chinese(const vector<int64_t>& b, const vector<int64_t>& primes) {
    int64_t p = 1, a = 0;
    for (int n = b.size(), i = 0; i < n; i++) {
        int64_t q = primes[i], x, y, g = gcd(p, q, x, y);
        (void)g, assert(g == 1);
        a = a * q * y + b[i] * p * x;
        p = p * q;
        a = a % p, a = a >= 0 ? a : a + p;
    }
    return a;
}

auto phi(int64_t n) {
    int64_t tot = 1;
    if ((n & 1) == 0) {
        n >>= 1;
        while ((n & 1) == 0)
            tot <<= 1, n >>= 1;
    }
    for (int64_t p = 3; p * p <= n; p += 2) {
        if (n % p == 0) {
            tot *= p - 1;
            n = n / p;
            while (n % p == 0) {
                tot *= p;
                n = n / p;
            }
        }
    }
    tot *= n > 1 ? n - 1 : 1;
    return tot;
}

auto factorize(int64_t n) {
    map<int64_t, int> primes;
    for (int64_t p = 2; p * p <= n; p++) {
        while (n % p == 0) {
            n /= p;
            primes[p]++;
        }
    }
    if (n > 1) {
        primes[n]++;
    }
    return primes;
}

auto get_divisors(const map<int64_t, int>& factors, bool one, bool self) {
    vector<int64_t> divs = {1};
    for (const auto& [p, e] : factors) {
        int D = divs.size();
        divs.resize(D * (e + 1));
        for (int n = 1; n <= e; n++) {
            for (int d = 0; d < D; d++) {
                divs[d + n * D] = divs[d + (n - 1) * D] * p;
            }
        }
    }
    if (!one) {
        divs.erase(begin(divs));
    }
    if (!self && !divs.empty()) {
        divs.erase(begin(divs) + (divs.size() - 1));
    }
    return divs;
}

// Compute first primitive root modulo prime p. Complexity: O(sqrt(p)) worst-case
int primitive_root_prime(int p) {
    if (p == 2 || p == 3)
        return p - 1;
    if (p == 167772161 || p == 469762049 || p == 998244353)
        return 3;
    if (p == 1000000007)
        return 5;
    assert(p % 2 == 1);

    int divs[20] = {2};
    int cnt = 1;
    int x = (p - 1) / 2; // phi(p)
    while (x % 2 == 0) {
        x /= 2;
    }
    for (int i = 3; i * i <= x; i += 2) {
        if (x % i == 0) {
            divs[cnt++] = i;
            do {
                x /= i;
            } while (x % i == 0);
        }
    }
    if (x > 1) {
        divs[cnt++] = x;
    }
    for (int g = 2;; g++) {
        bool ok = true;
        for (int i = 0; i < cnt; i++) {
            if (modpow(g, (p - 1) / divs[i], p) == 1) {
                ok = false;
                break;
            }
        }
        if (ok) {
            return g;
        }
    }
    __builtin_unreachable();
}

// Compute the jacobi (a/b) = ±1 (from quadratic reciprocity) O(log ab)
int jacobi(int64_t a, int64_t b) {
    assert(b & 1);
    int t = 1, r;
    a = a % b;
    while (a != 0) {
        while ((a & 1) == 0) {
            a >>= 1, r = b % 8;
            if (r == 3 || r == 5)
                t = -t;
        }
        swap(a, b);
        if ((a % 4) == 3 && (b % 4) == 3)
            t = -t;
        a = a % b;
    }
    return b == 1 ? t : 0;
}

// Determines if a is a quadratic residue mod p (p prime) O(log p)
bool is_quadratic_residue(int64_t a, int64_t p) {
    a = a % p, a = a >= 0 ? a : a + p;
    return a != 0 && modpow(a, (p - 1) / 2, p) == 1;
}

// Fast miller-rabin test for compositeness of n. O(log n)
bool miller_rabin(int64_t n) {
    if (n < 5)
        return n == 2 || n == 3;
    if (n % 2 == 0)
        return false;
    int r = __builtin_ctzll(n - 1);
    int64_t d = n >> r;

    for (int witness : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37}) {
        if (witness > n)
            break;
        if (witness == n)
            return true;
        auto x = modpow(int64_t(witness), d, n);
        if (x == 1 || x == n - 1)
            continue;
        for (int i = 0; i < r - 1 && x != n - 1; i++) {
            x = __int128_t(x) * __int128_t(x) % n;
        }
        if (x != n - 1)
            return false;
    }
    return true;
}
