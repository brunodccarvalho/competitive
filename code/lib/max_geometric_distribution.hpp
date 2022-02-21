#pragma once

#include <random>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

/**
 * Like geometric_distribution<IntType>, but generates only integers in [0,N).
 * Probability that number k in [0,N) is generated is proportional to (1-p)^k.
 */
template <typename IntType = int>
class max_geometric_distribution {
    static_assert(std::is_integral<IntType>::value,
                  "result_type must be an integral type");

  public:
    using result_type = IntType;

    struct param_type {
        using distribution_type = max_geometric_distribution<result_type>;
        friend class max_geometric_distribution<result_type>;

        explicit param_type(result_type n, double p = 0.5) : M_n(n), M_p(p) {
            assert(p > 0.0 && p < 1.0 && n > result_type(0));
            initialize();
        }

        double p() const { return M_p; }
        result_type n() const { return M_n; }

        friend bool operator==(const param_type& p1, const param_type& p2) {
            return p1.M_n == p2.M_n && p1.M_p == p2.M_p;
        }
        friend bool operator!=(const param_type& p1, const param_type& p2) {
            return !(p1 == p2);
        }

      private:
        void initialize() {
            M_log_1_p = std::log1p(-M_p);
            double largest = 1.0 - std::exp(M_log_1_p * M_n);
            M_uniform_dist = std::uniform_real_distribution<double>(0.0, largest);
        }

        result_type M_n;
        double M_p;
        double M_log_1_p;
        mutable std::uniform_real_distribution<double> M_uniform_dist;
    };

    explicit max_geometric_distribution(IntType n) : max_geometric_distribution(n, 0.5) {}

    max_geometric_distribution(IntType n, double p) : M_param(n, p) {}

    explicit max_geometric_distribution(double) = delete;

    explicit max_geometric_distribution(const param_type& p) : M_param(p) {}

    void reset() {}

    double p() const { return M_param.p(); }
    param_type param() const { return M_param; }
    void param(const param_type& param) { M_param = param; }

    result_type min() const { return 0; }
    result_type max() const { return M_param.n() - 1; }

    template <typename URNG>
    result_type operator()(URNG& urng) {
        return this->operator()(urng, M_param);
    }
    template <typename URNG>
    result_type operator()(URNG& urng, const param_type& p) {
        result_type cand = std::floor(std::log1p(-p.M_uniform_dist(urng)) / p.M_log_1_p);
        return std::min(cand, result_type(p.n() - 1));
    }

    friend bool operator==(const max_geometric_distribution& d1,
                           const max_geometric_distribution& d2) {
        return d1.M_param == d2.M_param;
    }
    friend bool operator!=(const max_geometric_distribution& d1,
                           const max_geometric_distribution& d2) {
        return d1.M_param != d2.M_param;
    }

  private:
    param_type M_param;
};

#pragma GCC diagnostic pop
