#pragma once

#include <bits/stdc++.h>
using namespace std;

inline void reverse_bits(unsigned& v) {
    v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
    v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
    v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
    v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
    v = (v >> 16) | (v << 16);
}

/**
 * FOR_EACH_MASK
 * Iterate through the masks with `size` set bits using the lowest `max_size` bits only.
 * - (out) mask: name for variable holding the generated mask
 * - (in)  size: number of set bits in mask.
 * - (in)  max_size: how many bit positions to iterate through
 * size and max_size should not be 0.
 */
inline void next_lexicographical_mask(unsigned& v) {
    unsigned c = v & -v, r = v + c;
    v = c ? (((r ^ v) >> 2) / c) | r : 0;
}
#define FOR_EACH_MASK(mask, size, max_size)                                            \
    for (unsigned mask = (1 << (size)) - 1, m##mask = 1 << (max_size); mask < m##mask; \
         next_lexicographical_mask(mask))

/**
 * Iterate through all masks over the lowest `max_size` bits, in gray-code order so that
 * each two consecutive masks differ in exactly 1 bit
 * - (out) mask: name for variable holding the generated mask
 * - (in)  max_size: how many bit positions to iterate through
 * - (out) in: bit added to mask in the current iteration (in=-1 or 0<=in<max_size)
 * - (out) out: bit removed from mask in the current iteration (out=-1 or 0<=out<max_size)
 *
 * Note: in/out are  0  2^(n-1) times,
 *       in/out are  1  2^(n-2) times,
 *       ...
 *       in/out are n-2 1 times,
 *       in     is  n-1 1 time.
 *
 * Usage:
 *   int n; // ...
 *   auto val = ...; // initialize for mask 0
 *   FOR_ALL_MASKS_GRAY_CODE(mask, n, in, out) {
 *       if (in != -1) {
 *           ... update val adding the bit 'in'
 *       } else if (out != -1) {
 *           ... update val removing the bit 'out'
 *       }
 *       ...
 *   }
 */
inline auto first_gray_code(int size, int max_size) {
    assert(0 <= size && size <= max_size && max_size <= 31);
    return make_tuple((1u << size) - 1, (1u << max_size) - 1, 0u, -1, -1);
}
inline void next_gray_code(unsigned& u, unsigned i, int& in, int& out) {
    assert(i > 0);
    unsigned b = u ^ i ^ (i >> 1);
    if (u & b) {
        in = -1, out = __builtin_ctz(b);
    } else {
        in = __builtin_ctz(b), out = -1;
    }
    u ^= b;
}
#define FOR_ALL_MASKS_GRAY_CODE(mask, max_size, in, out)                      \
    for (auto [mask, max##in, i##in, in, out] = first_gray_code(0, max_size); \
         i##in <= max##in; i##in++, next_gray_code(mask, i##in, in, out))

/**
 * Sized gray codes
 * Usage: By hand:
 *   int n; // ...
 *   auto grays = build_gray_code_per_size(n);
 *   for (int size = 0; size <= n; size++) {
 *     auto val = ...; // insert bits [0...size-1]
 *     ... process grays[size][0], val
 *     for (int g = 1, G = grays[size].size(); g < G; g++){
 *         auto mask = grays[size][g];
 *         auto [in, out] = get_in_out(grays[size][g - 1], mask);
 *         sum += a[in];  // insert in bit
 *         sum -= a[out]; // remove out bit
 *        ... process mask, val
 *     }
 *   }
 */
inline auto get_in_out(unsigned prev, unsigned next) {
    unsigned bits = prev ^ next;
    unsigned a = bits & (bits - 1), b = a ^ bits;
    int ai = __builtin_ctz(a), bi = __builtin_ctz(b);
    return (a & prev) ? make_pair(bi, ai) : make_pair(ai, bi);
}
auto build_gray_code_per_size(int n) {
    const int N = 1 << n;
    vector<vector<unsigned>> grays(N + 1);
    for (int i = 0; i < N; i++) {
        int u = i ^ (i >> 1);
        grays[__builtin_popcount(u)].push_back(u);
    }
    return grays;
}

/**
 * Iterate through the set bits of an unsigned integer mask.
 * - (out) bit: name for variable holding the shifted bit (1 << n)
 * - (out) i: the bit (0...i-1)
 * - (in)  mask: an unsigned integer mask (expression)
 * mask should not be 0.
 */
#define FOR_EACH_BIT(bit, mask)                                                      \
    for (remove_const<decltype(mask)>::type z##bit = (mask), bit = z##bit & -z##bit; \
         z##bit; z##bit ^= bit, z##bit && (bit = z##bit & -z##bit))

#define FOR_EACH_BIT_NUMBER(bit, i, mask)                                            \
    for (remove_const<decltype(mask)>::type z##bit = (mask), bit = z##bit & -z##bit, \
                                            i = __builtin_ctz(bit);                  \
         z##bit;                                                                     \
         z##bit ^= bit, z##bit && (bit = z##bit & -z##bit, i = __builtin_ctz(bit)))

/**
 * Format bit strings
 */
string dlsbits(unsigned v) {
    string s;
    while (v > 0) {
        s.push_back('0' + (v & 1));
        v >>= 1;
    }
    return s;
}

string dmsbits(uint64_t v) {
    string s;
    while (v > 0) {
        s.push_back('0' + (v & 1));
        v >>= 1;
    }
    reverse(begin(s), end(s));
    return s;
}

string lsbits(uint64_t v, unsigned bits = 64) {
    string s(bits, '0');
    for (unsigned i = 0; i < bits; i++) {
        s[i] = "01"[(v >> i) & 1];
    }
    return s;
}

string msbits(uint64_t v, unsigned bits = 64) {
    string s(bits, '0');
    for (unsigned i = 0; i < bits; i++) {
        s[bits - i - 1] = "01"[(v >> i) & 1];
    }
    return s;
}
