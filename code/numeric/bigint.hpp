#pragma once

#include "hash.hpp" // if necessary

/**
 * Bigint number over base 2^32
 * No Karatsuba multiplication yet unfortunately
 */
struct bigint {
    static_assert(0xffffffff == UINT_MAX);
    static_assert(sizeof(unsigned) == 4 && sizeof(ulong) == 8,
                  "Unexpected integer sizes");

    vector<unsigned> nums;
    bool sign = 0; // 0=positive, 1=negative

    bigint() = default;
    bigint(int n) : nums(n != 0, abs(n)), sign(n < 0) {}
    bigint(unsigned n, bool s = 0) : nums(n > 0, n), sign(s && n) {}
    bigint(const string& s, unsigned b = 10) {
        assert(2 <= b && b <= 10);
        int i = 0, S = s.size();
        while (i < S && isspace(s[i]))
            i++; // skip whitespace
        bool neg = i < S && s[i] == '-', pos = i < S && s[i] == '+';
        i += neg || pos, sign = neg;
        unsigned n = 0, tens = 1, threshold = UINT_MAX / (b + 1);
        while (i < S && ('0' <= s[i] && s[i] < char('0' + b))) {
            n = b * n + unsigned(s[i++] - '0');
            tens *= b;
            if (tens >= threshold) {
                mul_int(*this, tens);
                add_int(*this, n);
                n = 0;
                tens = 1;
            }
        }
        mul_int(*this, tens);
        add_int(*this, n);
    }

    auto& operator[](unsigned x) { return nums[x]; }
    const auto& operator[](unsigned x) const { return nums[x]; }
    bool bit(unsigned x) const { return nums[x / 32] & (1 << (x % 32)); }
    int len() const { return nums.size(); }
    bool zero() const noexcept { return nums.empty(); }
    void clear() { nums.clear(), sign = 0; }
    void flip() { sign = !sign && !zero(); }
    void trim() {
        while (!zero() && nums.back() == 0)
            nums.pop_back();
        sign = sign && !zero();
    }

    explicit operator bool() const noexcept { return !zero(); }
    explicit operator double() const noexcept {
        double v = 0, e = pow(2, 32);
        for (int i = 0; i < len(); i++)
            v = e * v + nums[i];
        return v;
    }

    friend bool magnitude_cmp(const bigint& u, const bigint& v) {
        int L = u.len(), R = v.len();
        return L != R ? L < R
                      : lexicographical_compare(rbegin(u.nums), rend(u.nums), //
                                                rbegin(v.nums), rend(v.nums));
    }
    friend bool operator<(const bigint& u, const bigint& v) {
        return u.sign != v.sign ? u.sign
               : u.sign         ? magnitude_cmp(v, u)
                                : magnitude_cmp(u, v);
    }
    friend bool operator>(const bigint& u, const bigint& v) { return v < u; }
    friend bool operator<=(const bigint& u, const bigint& v) { return !(u > v); }
    friend bool operator>=(const bigint& u, const bigint& v) { return !(u < v); }
    friend bool operator==(const bigint& u, const bigint& v) {
        return u.sign == v.sign && u.nums == v.nums;
    }
    friend bool operator!=(const bigint& u, const bigint& v) { return !(u == v); }

    friend bigint& operator>>=(bigint& u, unsigned shift) {
        int s = shift / 32, n = u.len();
        unsigned lo = shift % 32, hi = 32 - lo;

        if (s >= n) {
            u.clear();
        } else if (lo > 0) {
            for (int i = 0; i < n - s - 1; i++)
                u[i] = (u[i + s] >> lo) | (u[i + s + 1] << hi);
            u[n - s - 1] = u[n - 1] >> lo;
            u.nums.resize(n - s);
            u.trim();
        } else {
            u.nums.erase(begin(u.nums), begin(u.nums) + s);
        }

        return u;
    }
    friend bigint& operator<<=(bigint& u, unsigned shift) {
        int s = shift / 32, n = u.len();
        unsigned hi = shift % 32, lo = 32 - hi;

        if (hi > 0) {
            u.nums.resize(n + s + 1, 0);
            for (int i = n + s; i > s; i--)
                u[i] = (u[i - s - 1] >> lo) | (u[i - s] << hi);
            u[s] = u[0] << hi;
            for (int i = s - 1; i >= 0; i--)
                u[i] = 0;
            u.trim();
        } else {
            u.nums.insert(begin(u.nums), s, 0);
        }

        return u;
    }

    friend bigint operator>>(bigint u, unsigned shift) { return u >>= shift; }
    friend bigint operator<<(bigint u, unsigned shift) { return u <<= shift; }

    friend bigint& operator&=(bigint& u, const bigint& v) {
        int n = min(u.len(), v.len());
        u.nums.resize(n);
        for (int i = 0; i < n; i++)
            u[i] = u[i] & v[i];
        u.trim();
        return u;
    }
    friend bigint& operator|=(bigint& u, const bigint& v) {
        int n = max(u.len(), v.len());
        u.nums.resize(n, 0);
        for (int i = 0; i < v.len(); i++)
            u[i] = u[i] | v[i];
        return u;
    }
    friend bigint& operator^=(bigint& u, const bigint& v) {
        int n = max(u.len(), v.len());
        u.nums.resize(n, 0);
        for (int i = 0; i < v.len(); i++)
            u[i] = u[i] ^ v[i];
        u.trim();
        return u;
    }
    friend bigint operator~(bigint u) {
        for (int i = 0; i < u.len(); i++)
            u[i] = ~u[i];
        u.trim();
        return u;
    }

    friend bigint operator&(bigint u, const bigint& v) { return u &= v; }
    friend bigint operator|(bigint u, const bigint& v) { return u |= v; }
    friend bigint operator^(bigint u, const bigint& v) { return u ^= v; }

    friend string lsbits(const bigint& u) {
        if (u.zero())
            return "0";
        string s(32 * u.len() + 1, '0');
        s[0] = u.sign ? '-' : '+';
        for (int i = 0; i < 32 * u.len(); i++)
            s[i + 1] = '0' + u.bit(i);
        while (!s.empty() && s.back() == '0')
            s.pop_back();
        return s;
    }
    friend string msbits(const bigint& u) {
        if (u.zero())
            return "0";
        string s(32 * u.len() + 1, '0');
        s[0] = u.sign ? '-' : '+';
        for (int i = 0; i < 32 * u.len(); i++)
            s[32 * u.len() - i] = '0' + u.bit(i);
        s.erase(begin(s) + 1, find(begin(s) + 1, end(s), '1'));
        return s;
    }

    friend void add_int(bigint& u, unsigned v) {
        for (int i = 0; v && i < u.len(); i++)
            u[i] += v, v = u[i] < v;
        if (v > 0)
            u.nums.push_back(v);
    }
    friend void sub_int(bigint& u, unsigned v) {
        if (v == 0)
            return;
        if (u.zero()) {
            u.nums = {v}, u.sign = 1;
            return;
        }
        if (u.len() == 1 && u[0] < v) {
            u.nums = {v - u[0]}, u.flip();
            return;
        }
        for (int i = 0; v && i < u.len(); i++) {
            bool carry = v > u[i];
            u[i] -= v;
            v = carry;
        }
        u.trim();
    }
    friend void mul_int(bigint& u, unsigned v) {
        if (v == 0) {
            u.clear();
            return;
        }
        if (v == 1) {
            return;
        }
        ulong m = v, sum = 0;
        for (int i = 0; i < u.len(); i++) {
            sum += u[i] * m;
            u[i] = sum;
            sum >>= 32;
        }
        if (sum > 0)
            u.nums.push_back(sum);
    }
    friend unsigned div_int(bigint& u, unsigned v) {
        constexpr ulong b = 1UL + UINT_MAX;
        assert(v > 0);
        if (v == 1 || u.zero())
            return 0;
        unsigned r = 0;
        for (int i = u.len() - 1; i >= 0; i--) {
            ulong p = r * b + u[i];
            u[i] = p / v, r = p % v;
        }
        u.trim();
        return r;
    }
    friend void add_vec(bigint& u, const bigint& v) {
        int n = u.len(), m = v.len(), hi = max(n, m), lo = min(n, m);
        u.nums.resize(hi, 0);
        ulong k = 0;
        for (int i = 0; i < lo; i++) {
            k += u[i], k += v[i];
            u[i] = k, k = k > UINT_MAX;
        }
        for (int i = lo; i < m; i++) {
            k += v[i];
            u[i] = k, k = k > UINT_MAX;
        }
        for (int i = m; k && i < hi; i++) {
            u[i]++, k = u[i] == 0;
        }
        if (k)
            u.nums.push_back(k);
    }
    friend void sub_vec(bigint& u, const bigint& v) {
        int n = u.len(), m = v.len();
        assert(n >= m);
        long k = 0;
        for (int i = 0; i < m; i++) {
            long sum = u[i] - k - v[i];
            u[i] = sum + UINT_MAX + 1;
            k = sum < 0;
        }
        for (int i = m; k && i < n; i++) {
            k = u[i] == 0;
            u[i]--;
        }
        assert(k == 0);
        u.trim();
    }
    friend void rev_sub_vec(bigint& u, const bigint& v) {
        int n = u.len(), m = v.len();
        assert(n <= m);
        u.nums.resize(m, 0);
        long k = 0;
        for (int i = 0; i < n; i++) {
            long sum = v[i] - k - u[i];
            u[i] = sum + UINT_MAX + 1;
            k = sum < 0;
        }
        for (int i = n; i < m; i++) {
            u[i] = v[i];
        }
        for (int i = n; k && i < m; i++) {
            k = v[i] == 0;
            u[i]--;
        }
        assert(k == 0);
        u.trim(), u.flip();
    }
    friend void dyn_sub_vec(bigint& u, const bigint& v) {
        int n = u.len(), m = v.len();
        if (n > m) {
            return sub_vec(u, v);
        } else if (n < m) {
            return rev_sub_vec(u, v);
        } else {
            int i = n - 1;
            while (i >= 0 && u[i] == v[i])
                u.nums.pop_back(), i--;
            if (i < 0) {
                u.sign = 0;
                return;
            }
            n = i + 1;
            long k = 0;
            if (u[i] > v[i]) {
                for (i = 0; i < n; i++) {
                    long sum = u[i] - k - v[i];
                    u[i] = sum + UINT_MAX + 1;
                    k = sum < 0;
                }
            } else {
                for (i = 0; i < n; i++) {
                    long sum = v[i] - k - u[i];
                    u[i] = sum + UINT_MAX + 1;
                    k = sum < 0;
                }
                u.flip();
            }
            u.trim();
            assert(k == 0);
        }
    }
    friend bigint mul_vec(const bigint& u, const bigint& v) {
        if (u.zero() || v.zero())
            return 0;
        int n = u.len(), m = v.len();
        bigint c;
        c.nums.resize(n + m, 0);
        c.sign = u.sign ^ v.sign;
        for (int j = 0; j < m; j++) {
            unsigned k = 0;
            for (int i = 0; i < n; i++) {
                ulong t = ulong(u[i]) * v[j] + c[i + j] + k;
                c[i + j] = t & UINT_MAX;
                k = t >> 32;
            }
            c[n + j] = k;
        }
        c.trim();
        return c;
    }
    friend bigint div_vec(bigint& u, bigint v) {
        constexpr ulong b = 1L + UINT_MAX;
        assert(!v.zero());

        // return the remainder and set u to the quotient, but throughout the algorithm
        // u is the remainder and d is the quotient.
        int n = v.len(), m = u.len() - n;
        unsigned c = __builtin_clz(v[n - 1]);
        u <<= c, v <<= c;
        if (u.len() == n + m)
            u.nums.push_back(0);
        assert(u.len() == n + m + 1 && v.len() == n && v[n - 1] >= b / 2);

        bigint d;
        d.nums.resize(m + 1, 0);
        d.sign = u.sign ^ v.sign;

        for (int j = m; j >= 0; j--) {
            ulong q = (u[n + j] * b + u[n - 1 + j]) / v[n - 1];
            ulong r = (u[n + j] * b + u[n - 1 + j]) % v[n - 1];

            while (r < b && q * v[n - 2] > r * b + u[n - 2 + j])
                q--, r += v[n - 1];
            if (q == 0)
                continue;

            long k = 0, t;
            for (int i = 0; i < n; i++) {
                ulong p = q * v[i];
                t = u[i + j] - (p & UINT_MAX) - k;
                u[i + j] = t;
                k = (p >> 32) - (t >> 32);
            }
            u[j + n] = t = u[j + n] - k;

            d[j] = q;
            if (t < 0) {
                assert(q > 0 && t == -1);
                d[j]--;
                k = 0;
                for (int i = 0; i < n; i++) {
                    t = u[i + j] + k + v[i];
                    u[i + j] = t;
                    k = t > UINT_MAX;
                }
                u[j + n] = u[j + n] + k;
            }
            assert(u[j + n] == 0);
        }

        u.trim(), u >>= c, d.trim();
        swap(u, d);
        return d;
    }
    friend bigint div_mod(bigint& u, const bigint& v) {
        bigint r;
        if (magnitude_cmp(u, v)) {
            r = move(u);
            u.clear();
        } else if (v.len() == 1) {
            r = bigint(div_int(u, v[0]), u.sign);
            u.sign ^= v.sign, r.sign &= !r.zero();
        } else {
            r = div_vec(u, v);
        }
        return r;
    }

    friend bigint& operator+=(bigint& u, const bigint& v) {
        u.sign == v.sign ? add_vec(u, v) : dyn_sub_vec(u, v);
        return u;
    }
    friend bigint& operator-=(bigint& u, const bigint& v) {
        u.sign != v.sign ? add_vec(u, v) : dyn_sub_vec(u, v);
        return u;
    }
    friend bigint& operator*=(bigint& u, const bigint& v) {
        u = mul_vec(u, v);
        return u;
    }
    friend bigint& operator/=(bigint& u, const bigint& v) {
        div_mod(u, v);
        return u;
    }
    friend bigint& operator%=(bigint& u, const bigint& v) {
        u = div_mod(u, v);
        return u;
    }

    friend bigint& operator+=(bigint& u, unsigned n) {
        u.sign == 0 ? add_int(u, n) : sub_int(u, n);
        return u;
    }
    friend bigint& operator-=(bigint& u, unsigned n) {
        u.sign == 1 ? add_int(u, n) : sub_int(u, n);
        return u;
    }
    friend bigint& operator*=(bigint& u, unsigned n) {
        mul_int(u, n);
        return u;
    }
    friend bigint& operator/=(bigint& u, unsigned n) {
        div_int(u, n);
        return u;
    }
    friend bigint& operator%=(bigint& u, unsigned n) {
        u = bigint(div_int(u, n), u.sign);
        return u;
    }

    friend bigint& operator+=(bigint& u, int n) {
        n >= 0 ? u += unsigned(n) : u -= unsigned(abs(n));
        return u;
    }
    friend bigint& operator-=(bigint& u, int n) {
        n >= 0 ? u -= unsigned(n) : u += unsigned(abs(n));
        return u;
    }
    friend bigint& operator*=(bigint& u, int n) {
        mul_int(u, abs(n)), u.sign ^= n < 0;
        return u;
    }
    friend bigint& operator/=(bigint& u, int n) {
        div_int(u, abs(n)), u.sign ^= n < 0;
        return u;
    }
    friend bigint& operator%=(bigint& u, int n) {
        u = bigint(div_int(u, abs(n)), u.sign);
        return u;
    }

    friend bigint operator+(bigint u, const bigint& v) { return u += v; }
    friend bigint operator+(bigint u, unsigned n) { return u += n; }
    friend bigint operator+(bigint u, int n) { return u += n; }
    friend bigint operator+(unsigned n, bigint u) { return u += n; }
    friend bigint operator+(int n, bigint u) { return u += n; }

    friend bigint operator-(bigint u, const bigint& v) { return u -= v; }
    friend bigint operator-(bigint u, unsigned n) { return u -= n; }
    friend bigint operator-(bigint u, int n) { return u -= n; }

    friend bigint operator*(const bigint& u, const bigint& v) { return mul_vec(u, v); }
    friend bigint operator*(bigint u, unsigned n) { return u *= n; }
    friend bigint operator*(bigint u, int n) { return u *= n; }
    friend bigint operator*(unsigned n, bigint u) { return u *= n; }
    friend bigint operator*(int n, bigint u) { return u *= n; }

    friend bigint operator/(bigint u, const bigint& v) { return u /= v; }
    friend bigint operator/(bigint u, unsigned n) { return u /= n; }
    friend bigint operator/(bigint u, int n) { return u /= n; }

    friend bigint operator%(bigint u, const bigint& v) { return u %= v; }
    friend bigint operator%(bigint u, unsigned n) { return u %= n; }
    friend bigint operator%(bigint u, int n) { return u %= n; }

    bigint operator++() { return *this += 1u; }
    bigint operator--() { return *this -= 1u; }
    bigint operator++(int) {
        auto b = *this;
        return ++*this, b;
    }
    bigint operator--(int) {
        auto b = *this;
        return --*this, b;
    }

    friend bigint operator-(bigint u) { return u.flip(), u; }
    friend bool operator!(const bigint& u) { return u.zero(); }

    friend bigint abs(const bigint& u) { return u.sign ? -u : u; }
    friend bigint gcd(bigint a, bigint b) {
        while (a) {
            b = b % a;
            swap(a, b);
        }
        return abs(b);
    }
    friend bigint sqrt(const bigint& u) {
        assert(u.sign == 0); // no complex fuckery here
        int n = u.len(), m = (n + 1) / 2;
        if (n == 0) {
            return 0;
        } else if (n == 1) {
            return unsigned(std::sqrt(u[0]));
        } else if (n == 2) {
            return unsigned(std::sqrt(ulong(u[1]) << 32 | u[0]));
        }
        bigint x, y;
        x.nums.resize(m);
        x.nums[m - 1] = std::sqrt(ulong(u[n - 1]) << 32 | u[n - 2]);
        do { // iterate newton until stabilization
            y = move(x), x = (y + u / y) >> 1;
        } while (x != y);
        return x;
    }

    friend string to_string(bigint u, unsigned b = 10) {
        static auto uint_to_string = [](unsigned n, unsigned base) {
            string s;
            while (n > 0) {
                unsigned m = n / base;
                s += '0' + (n - base * m), n = m;
            }
            reverse(begin(s), end(s));
            return s;
        };

        if (u.zero())
            return "0";
        string s = u.sign ? "-" : "";
        vector<string> rems;
        unsigned divisor = b, digits = 1;
        while (divisor < UINT_MAX / b) {
            divisor *= b, digits++;
        }
        while (!u.zero()) {
            rems.push_back(uint_to_string(div_int(u, divisor), b));
        }
        for (int i = 0, n = rems.size(); i < n; i++) {
            string pad(i ? digits - rems[n - i - 1].length() : 0, '0');
            s += pad + rems[n - i - 1];
        }
        return s;
    }

    friend ostream& operator<<(ostream& out, const bigint& u) {
        return out << to_string(u);
    }
};

namespace std {

template <>
struct hash<bigint> {
    size_t operator()(const bigint& u) const noexcept {
        return std::hash<pair<vector<unsigned>, bool>>{}(make_pair(u.nums, u.sign));
    }
};

} // namespace std
