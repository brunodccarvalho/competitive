#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T>
struct Binomial {
    static T fac(int n) {
        ensure_factorial(n);
        return n < 0 ? 0 : fact[n];
    }

    static T invfac(int n) {
        ensure_factorial(n);
        return n < 0 ? 0 : ifact[n];
    }

    static T choose(int n, int k) {
        ensure_factorial(n);
        return n < 0 || k < 0 || k > n ? 0 : fact[n] * ifact[k] * ifact[n - k];
    }

    static T lah(int n, int k) {
        ensure_factorial(n);
        return n <= 0 || k <= 0 || k > n ? 0 : choose(n - 1, k - 1) * fac[n] * ifact[k];
    }

    static T choose(const vector<int>& ks) {
        int n = 0;
        for (int k : ks) {
            n += k;
            if (k < 0) {
                return 0;
            }
        }
        ensure_factorial(n);
        T ans = fact[n];
        for (int k : ks) {
            ans *= ifact[k];
        }
        return ans;
    }

    static T multinomial(const vector<int>& ks) { return choose(ks); }

    // Partial factorial n(n+1)...(n+k-1), with 1 for k=0 and n>=1
    static T rising(int n, int k) {
        ensure_factorial(n + k - 1);
        return n <= 0 || k < 0 ? 0 : fact[n + k - 1] * ifact[n - 1];
    }

    // Partial factorial n(n-1)...(n-k+1), with 1 for k=0 and n>=0
    static T falling(int n, int k) { // 1 for k=0, same as arrange(n, k)
        ensure_factorial(n);
        return n < 0 || k < 0 || k > n ? 0 : fact[n] * ifact[n - k];
    }

    // Layout n identical items over k distinct bins, >=a per bin
    static T layout(int n, int k, int a = 0) {
        return k == 0 ? n == 0 : choose(n + (1 - a) * k - 1, k - 1);
    }

    // Layout n identical items over k distinct bins, >=a and <=b per bin. O(k)
    static T bounded_layout(int n, int k, int a, int b) {
        n -= a * k, b -= a;
        T ans = 0; // inclusion-exclusion
        for (int i = 0; i <= k && b * i <= n && b > 0; i++) {
            ans += (i % 2 ? -1 : +1) * choose(k, i) * layout(n - (b + 1) * i, k);
        }
        return b == 0 ? n == 0 : ans;
    }

    static T catalan(int n) {
        ensure_factorial(2 * n + 1);
        return n < 0 ? 0 : fact[2 * n] * ifact[n] * ifact[n + 1];
    }

    // Σ{i1+...+ik=n} ∏ catalan(ij)
    static T catalan_conv(int n, int k) {
        ensure_factorial(2 * n + k);
        return n < 0 || k < 1 ? 0 : choose(2 * n + k - 1, n) * k * inv(n + k);
    }

    // Chance of k successes in a binomial(n,p) (n events, success with probability p)
    static T binomial_success(int n, int k, T p) {
        return n < 0 || k < 0 ? 0 : choose(n, k) * binpow(p, k) * binpow(1 - p, n - k);
    }

    static T binpow(T v, int64_t e) {
        T ans = 1;
        while (e > 0) {
            if (e & 1)
                ans *= v;
            if (e >>= 1)
                v *= v;
        }
        return ans;
    }

    static T inv(int n) {
        ensure_factorial(n);
        return n == 0 ? 0 : n < 0 ? -inv(-n) : fact[n - 1] * ifact[n];
    }

    // * Cache
    static inline vector<T> fact = {1, 1};
    static inline vector<T> ifact = {1, 1};

    static void ensure_factorial(int n) {
        if (int m = fact.size(); n >= m) {
            n = 1 << (8 * sizeof(int) - __builtin_clz(n - 1));
            fact.resize(n + 1);
            ifact.resize(n + 1);
            for (int i = m; i <= n; i++) {
                fact[i] = T(i) * fact[i - 1];
            }
            ifact[n] = T(1) / fact[n];
            for (int i = n; i > m; i--) {
                ifact[i - 1] = T(i) * ifact[i];
            }
        }
    }
};

template <typename T>
struct Partition {
    // Count of unrestricted partitions of n (>=1) (check complexity)
    static T partitions(int n) {
        ensure_partition(n);
        return n < 0 ? 0 : partp[n];
    }

    // Partitions with distinct parts
    static T partitions_distinct(int n) {
        ensure_partition_distinct(n);
        return n < 0 ? 0 : partq[n];
    }

    // Partitions with distinct odd-sized parts (self-conjugate)
    static T partitions_distinct_odd(int n) {
        ensure_partition_odd_distinct(n);
        return n < 0 ? 0 : partd[n];
    }

    // Partitions with distinct even-sized parts
    static T partitions_distinct_even(int n) {
        return (n % 2) ? 0 : partitions_distinct(n / 2);
    }

    // Partitions with odd-sized parts
    static T partitions_odd(int n) { return partitions_distinct(n); }

    // Partitions with even-sized parts
    static T partitions_even(int n) { return (n % 2) ? 0 : partitions(n / 2); }

    // Cache
    static inline vector<T> partp = {1, 1};
    static inline vector<T> partq = {1, 1};
    static inline vector<T> partd = {1, 1};

    static void ensure_partition(int n) {
        if (int m = partp.size(); n >= m) {
            n = 1 << (8 * sizeof(int) - __builtin_clz(n - 1));
            partp.resize(n + 1);
            for (int i = m; i <= n; i++) {
                for (int k = 1; k * (3 * k - 1) / 2 <= i; k += 2)
                    partp[i] += partp[i - k * (3 * k - 1) / 2];
                for (int k = 2; k * (3 * k - 1) / 2 <= i; k += 2)
                    partp[i] -= partp[i - k * (3 * k - 1) / 2];
                for (int k = 1; k * (3 * k + 1) / 2 <= i; k += 2)
                    partp[i] += partp[i - k * (3 * k + 1) / 2];
                for (int k = 2; k * (3 * k + 1) / 2 <= i; k += 2)
                    partp[i] -= partp[i - k * (3 * k + 1) / 2];
            }
        }
    }

    static void ensure_partition_distinct(int n) {
        static int plus = 1, minus = 2;
        if (int m = partq.size(); n >= m) {
            n = 1 << (8 * sizeof(int) - __builtin_clz(n - 1));
            partq.resize(n + 1);
            for (int i = m; i <= n; i++) {
                for (int k = 1; k * k <= i; k += 2)
                    partq[i] += 2 * partq[i - k * k];
                for (int k = 2; k * k <= i; k += 2)
                    partq[i] -= 2 * partq[i - k * k];

                if (i == minus * (3 * minus - 1) / 2) {
                    partq[i] += minus++ % 2 ? -1 : +1;
                } else if (i == plus * (3 * plus + 1) / 2) {
                    partq[i] += plus++ % 2 ? -1 : +1;
                }
            }
        }
    }

    static void ensure_partition_odd_distinct(int n) {
        static int plus = 1, minus = 1;
        if (int m = partd.size(); n >= m) {
            n = 1 << (8 * sizeof(int) - __builtin_clz(n - 1));
            partd.resize(n + 1);
            for (int i = m; i <= n; i++) {
                for (int k = 1; k * (k + 1) / 2 <= i; k += 4)
                    partd[i] += partd[i - k * (k + 1) / 2];
                for (int k = 2; k * (k + 1) / 2 <= i; k += 4)
                    partd[i] += partd[i - k * (k + 1) / 2];
                for (int k = 3; k * (k + 1) / 2 <= i; k += 4)
                    partd[i] -= partd[i - k * (k + 1) / 2];
                for (int k = 4; k * (k + 1) / 2 <= i; k += 4)
                    partd[i] -= partd[i - k * (k + 1) / 2];

                if (i == minus * (3 * minus - 1)) {
                    partd[i] += minus++ % 2 ? -1 : +1;
                } else if (i == plus * (3 * plus + 1)) {
                    partd[i] += plus++ % 2 ? -1 : +1;
                }
            }
        }
    }
};

template <typename T>
struct Permutations {
    using B = Binomial<T>;

    // Number of derangements (permutations with no fixed points) of length n
    static T derangements(int n) {
        ensure_derangements(n);
        return n < 0 ? 0 : deran[n];
    }

    // Number of permutations with k fixed points
    static T derangements(int n, int k) {
        B::ensure_factorial(n), ensure_derangements(n);
        return n < 0 ? 0 : B::choose(n, k) * deran[n - k];
    }

    // Number of involutions of size n
    static T involutions(int n) {
        ensure_involutions(n);
        return n < 0 ? 0 : invol[n];
    }

    // Number of partitions of a set of n elements (check complexity)
    static T set_partitions(int n) {
        ensure_bell(n);
        return n < 0 ? 0 : belln[n];
    }

    // Number of permutations on n with k cycles
    static T stirling_1st(int n, int k) {
        ensure_stir1st(n);
        return n < 0 || k < 0 || k > n ? 0 : stir1st[n][k];
    }

    // Number of ways to layout n labeled elements into k unlabelled nonempty boxes
    static T stirling_2nd(int n, int k) {
        ensure_stir2nd(n);
        return n < 0 || k < 0 || k > n ? 0 : stir2nd[n][k];
    }

    // Number of ways to layout n labeled elements into k labelled nonempty boxes
    static T compositions(int n, int k) {
        ensure_stir2nd(n);
        return n < 0 || k < 0 || k > n ? 0 : Binomial<T>::fac(k) * stir2nd[n][k];
    }

    // Cache
    static inline vector<T> deran = {1, 0};
    static inline vector<T> belln = {1, 1};
    static inline vector<T> invol = {1, 1};
    static inline vector<vector<T>> stir1st = {{1}};
    static inline vector<vector<T>> stir2nd = {{1}};

    static void ensure_derangements(int n) {
        if (int m = deran.size(); n >= m) {
            n = 1 << (8 * sizeof(int) - __builtin_clz(n - 1));
            deran.resize(n + 1);
            for (int i = m; i <= n; i++) {
                deran[i] = (i - 1) * (deran[i - 1] + deran[i - 2]);
            }
        }
    }

    static void ensure_bell(int n) {
        if (int m = belln.size(); n >= m) {
            n = 1 << (8 * sizeof(int) - __builtin_clz(n - 1));
            B::ensure_factorial(n);
            belln.resize(n + 1);
            for (int i = m; i <= n; i++) {
                for (int k = 0; k < i; k++) {
                    belln[i] += B::choose(i - 1, k) * belln[k];
                }
            }
        }
    }

    static void ensure_involutions(int n) {
        if (int m = belln.size(); n >= m) {
            n = 1 << (8 * sizeof(int) - __builtin_clz(n - 1));
            invol.resize(n + 1);
            for (int i = m; i <= n; i++) {
                invol[i] = invol[i - 1] + T(i - 1) * invol[i - 2];
            }
        }
    }

    static void ensure_stir1st(int n) {
        if (int m = stir1st.size(); n >= m) {
            n = 1 << (8 * sizeof(int) - __builtin_clz(n - 1));
            stir1st.resize(n + 1);
            for (int i = m; i <= n; i++) {
                stir1st[i].resize(i + 1), stir1st[i][i] = 1;
                for (int k = 1; k < i; k++) {
                    stir1st[i][k] = (i - 1) * stir1st[i - 1][k] + stir1st[i - 1][k - 1];
                }
            }
        }
    }

    static void ensure_stir2nd(int n) {
        if (int m = stir2nd.size(); n >= m) {
            n = 1 << (8 * sizeof(int) - __builtin_clz(n - 1));
            stir2nd.resize(n + 1);
            for (int i = m; i <= n; i++) {
                stir2nd[i].resize(i + 1), stir2nd[i][i] = 1;
                for (int k = 1; k < i; k++) {
                    stir2nd[i][k] = k * stir2nd[i - 1][k] + stir2nd[i - 1][k - 1];
                }
            }
        }
    }
};
