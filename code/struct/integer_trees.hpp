#pragma once

#include <bits/stdc++.h>
using namespace std;

// Reference: https://github.com/Aeren1564/Algorithms color processor

/**
 * A "color map", associate with each run a "color". Merge runs with the same color
 * together, and splice runs that get cut, duplicating their color.
 * The map is initialized with the infimum of the universe and a default color.
 *
 * Complexity: O(log N) amortized per write
 */
template <typename T, typename V>
struct merging_runs_map : map<T, V> {
    using base_t = map<T, V>;

    // start must be an infimum of the universe of intervals
    merging_runs_map(T start, V init) { base_t::emplace(start, move(init)); }

    array<T, 2> universe() const {
        return {base_t::begin()->first, base_t::rbegin()->first};
    }

    // Write a new value over the interval [L,R)
    void write(T L, T R, V value) {
        auto hi = base_t::lower_bound(R);
        bool hit_hi = hi != base_t::end() && R >= hi->first;
        if (!hit_hi && value != prev(hi)->second) {
            hi = base_t::emplace_hint(hi, R, prev(hi)->second);
        } else if (hit_hi && value == hi->second) {
            ++hi;
        }
        auto lo = base_t::lower_bound(L);
        bool hit_lo = lo != base_t::end() && L >= lo->first;
        if (!hit_lo && value != prev(lo)->second) {
            lo = base_t::emplace_hint(lo, L, value), ++lo;
        } else if (hit_lo && (lo == base_t::begin() || value != prev(lo)->second)) {
            lo->second = value, ++lo;
        }
        base_t::erase(lo, hi);
    }

    V& val(T x) const { return std::prev(base_t::upper_bound(x))->second; }

    vector<tuple<T, T, V>> extract_runs(T L, T R) const {
        auto lo = prev(base_t::upper_bound(L));
        auto hi = base_t::lower_bound(R);
        vector<tuple<T, T, V>> blocks;
        auto a = lo;
        for (auto b = next(lo); b != hi; ++a, ++b) {
            blocks.emplace_back(max(a->first, L), b->first, a->second);
        }
        blocks.emplace_back(max(a->first, L), R, a->second);
        return blocks;
    }

    vector<V> extract_vector(T L, T R) const {
        vector<V> arr(R - L);
        auto lo = prev(base_t::upper_bound(L));
        auto hi = base_t::lower_bound(R);
        auto a = lo;
        for (auto b = next(lo); b != hi; ++a, ++b) {
            for (int i = max(a->first, L); i < b->first; i++) {
                arr[i - L] = a->second;
            }
        }
        for (int i = max(a->first, L); i < R; i++) {
            arr[i - L] = a->second;
        }
        return arr;
    }
};

/**
 * A run set with gaps. Maintain disjoint intervals, merge overlapping or adjacent
 * intervals on additions, and splice/remove/trim intervals that overlap with cuts.
 *
 * Complexity: O(log N) amortized per add/cut
 */
template <typename T>
struct merging_interval_set : set<array<T, 2>> {
    static constexpr inline T inf = numeric_limits<T>::max();
    using run_t = array<T, 2>;
    using base_t = set<run_t>;

    run_t universe() const { return {(*base_t::begin())[0], (*base_t::rbegin())[1]}; }

    void add(run_t intv) {
        auto& [L, R] = intv;
        auto lo = base_t::lower_bound({L, L});
        auto hi = base_t::upper_bound({R, inf});
        if (hi != base_t::begin()) { // extend to the right
            R = max((*prev(hi))[1], R);
        }
        if (lo != base_t::begin() && (*prev(lo))[1] >= L) { // extend to the left
            L = min((*--lo)[0], L);
        }
        base_t::erase(lo, hi);
        base_t::insert(hi, intv);
    }

    void cut(run_t intv) {
        auto& [L, R] = intv;
        auto lo = base_t::lower_bound({L, L});
        auto hi = base_t::lower_bound({R, R});
        if (hi != base_t::begin() && (*prev(hi))[0] < L && (*prev(hi))[1] > R) { // splice
            auto [x, y] = *prev(hi);
            base_t::erase(prev(hi));
            base_t::insert(hi, {x, L});
            base_t::insert(hi, {R, y});
            return;
        }
        if (hi != base_t::begin() && (*prev(hi))[1] > R) { // cut old rightwards
            const_cast<T&>((*--hi)[0]) = R;
        }
        if (lo != base_t::begin() && (*prev(lo))[1] > L) { // cut old leftwards
            const_cast<T&>((*prev(lo))[1]) = L;
        }
        base_t::erase(lo, hi);
    }

    optional<run_t> get_run(T x) const {
        auto it = base_t::upper_bound({x, inf});
        if (it != base_t::begin() && (*prev(it))[0] <= x && x < (*prev(it))[1]) {
            return *prev(it);
        }
        return std::nullopt;
    }

    bool contains(T x) const {
        auto it = base_t::upper_bound({x, inf});
        return it != base_t::begin() && (*prev(it))[0] <= x && x < (*prev(it))[1];
    }

    bool contains(run_t intv) const {
        auto wrap = get_run(intv[0]);
        return wrap.has_value() && (*wrap)[1] >= intv[1];
    }

    bool overlaps(run_t intv) const {
        auto lo = base_t::lower_bound({intv[0], intv[0]});
        return !(lo == base_t::end() || (*lo)[0] >= intv[1]) ||
               !(lo == base_t::begin() || (*--lo)[1] <= intv[0]);
    }

    // Get run including x, or the one closest before x (or end() if none)
    auto before(T x) const {
        auto it = base_t::upper_bound({x, inf});
        return it != base_t::begin() ? prev(it) : base_t::end();
    }

    // Get run strictly before x (or end() if none)
    auto before_strict(T x) const {
        auto it = base_t::upper_bound({x, x});
        if (it != base_t::begin() && prev(it)->first[1] > x)
            --it;
        return it != base_t::begin() ? prev(it) : base_t::end();
    }

    // Get run including x, or the one closest after x (or end() if none)
    auto after(T x) const {
        auto it = base_t::upper_bound({x, inf});
        return it != base_t::begin() && prev(it)->first[1] > x ? prev(it) : it;
    }

    // Get run strictly after x (or end() if none)
    auto after_strict(T x) const {
        return base_t::upper_bound({x, inf}); //
    }
};

/**
 * A run map with gaps, where each interval has associated a "color". Maintain disjoint
 * intervals, merge overlapping or adjacent intervals with the same color on additions,
 * and splice/remove/trim intervals that overlap with cuts.
 *
 * Complexity: O(log N) amortized per write/cut
 */
template <typename T, typename V>
struct merging_interval_map : map<array<T, 2>, V> {
    static constexpr inline T inf = numeric_limits<T>::max();
    using run_t = array<T, 2>;
    using base_t = map<array<T, 2>, V>;

    run_t universe() const {
        return {base_t::begin()->first[0], base_t::rbegin()->first[1]};
    }

    void write(run_t intv, V color) {
        auto& [L, R] = intv;
        if (L >= R) {
            return;
        }
        auto lo = base_t::lower_bound({L, L});
        auto hi = base_t::upper_bound({R, inf});
        if (hi != base_t::begin()) {
            if (auto phi = prev(hi); phi->first[1] > R) {
                if (phi->second == color) { // extend new rightwards, removing old
                    R = phi->first[1];
                } else if (phi->first[0] < L) { // splice
                    auto [x, y] = phi->first;
                    auto red = phi->second;
                    base_t::erase(phi);
                    base_t::emplace_hint(hi, run_t{x, L}, red);
                    base_t::emplace_hint(hi, run_t{L, R}, color);
                    base_t::emplace_hint(hi, run_t{R, y}, red);
                    return;
                } else { // cut old rightwards and don't remove it
                    const_cast<T&>(phi->first[0]) = R, hi = phi;
                }
            }
        }
        if (lo != base_t::begin()) {
            if (auto plo = prev(lo); plo->first[1] >= L) {
                if (plo->second == color) { // extend new leftwards, removing old
                    L = plo->first[0], lo = plo;
                } else if (plo->first[0] < L) { // cut old leftwards and don't remove it
                    const_cast<T&>(plo->first[1]) = L;
                } else { // just cut old
                    lo = plo;
                }
            }
        }
        base_t::erase(lo, hi);
        base_t::emplace_hint(hi, intv, color);
    }

    void cut(run_t intv) {
        auto& [L, R] = intv;
        if (L >= R) {
            return;
        }
        auto lo = base_t::lower_bound({L, L});
        auto hi = base_t::lower_bound({R, R});
        if (hi != base_t::begin()) {
            if (auto phi = prev(hi); phi->first[0] < L && phi->first[1] > R) { // splice
                auto [x, y] = phi->first;
                auto color = phi->second;
                base_t::erase(phi);
                base_t::emplace_hint(hi, run_t{x, L}, color);
                base_t::emplace_hint(hi, run_t{R, y}, color);
                return;
            } else if (phi->first[1] > R) { // cut old rightwards and don't remove it
                const_cast<T&>(phi->first[0]) = R, hi = phi;
            }
        }
        if (lo != base_t::begin()) {
            if (auto plo = prev(lo); plo->first[1] > L) { // cut old leftwards
                const_cast<T&>(plo->first[1]) = L;
            }
        }
        base_t::erase(lo, hi);
    }

    optional<run_t> get_run(T x) const {
        auto it = base_t::upper_bound({x, inf});
        if (it != base_t::begin() && prev(it)->first[0] <= x && x < prev(it)->first[1]) {
            return prev(it)->first;
        }
        return std::nullopt;
    }

    optional<V> get(T x) const {
        auto it = base_t::upper_bound({x, inf});
        if (it != base_t::begin() && prev(it)->first[0] <= x && x < prev(it)->first[1]) {
            return prev(it)->second;
        }
        return std::nullopt;
    }

    V& val(T x) { return prev(base_t::upper_bound({x, inf}))->second; }

    bool contains(T x) const { return get_run(x).has_value(); }

    bool contains(run_t intv) const {
        auto wrap = get_run(intv[0]);
        return wrap.has_value() && wrap->first[1] >= intv[1];
    }

    bool overlaps(run_t intv) const {
        auto lo = base_t::lower_bound({intv[0], intv[0]});
        return !(lo == base_t::end() || lo->first[0] >= intv[1]) ||
               !(lo == base_t::begin() || prev(lo)->first[1] <= intv[0]);
    }

    // Get run including x, or the one closest before x (or end() if none)
    auto before(T x) const {
        auto it = base_t::upper_bound({x, inf});
        return it != base_t::begin() ? prev(it) : base_t::end();
    }

    // Get run strictly before x (or end() if none)
    auto before_strict(T x) const {
        auto it = base_t::upper_bound({x, x});
        if (it != base_t::begin() && prev(it)->first[1] > x)
            --it;
        return it != base_t::begin() ? prev(it) : base_t::end();
    }

    // Get run including x, or the one closest after x (or end() if none)
    auto after(T x) const {
        auto it = base_t::upper_bound({x, inf});
        return it != base_t::begin() && prev(it)->first[1] > x ? prev(it) : it;
    }

    // Get run strictly after x (or end() if none)
    auto after_strict(T x) const {
        return base_t::upper_bound({x, inf}); //
    }
};

/**
 * Likewise, but report which colored intervals were overwritten or cut.
 * Processor is called as processor({L, R}, old_color)
 *
 * Complexity: O(log N) amortized per write/cut
 */
template <typename T, typename V>
struct merging_interval_processor : map<array<T, 2>, V> {
    static constexpr inline T inf = numeric_limits<T>::max();
    using run_t = array<T, 2>;
    using base_t = map<array<T, 2>, V>;

    run_t universe() const {
        return {base_t::begin()->first[0], base_t::rbegin()->first[1]};
    }

    template <typename Fn>
    void write(run_t intv, V color, Fn&& processor) {
        auto& [L, R] = intv;
        if (L >= R) {
            return;
        }
        auto lo = base_t::lower_bound({L, L});
        auto hi = base_t::upper_bound({R, inf});
        if (hi != base_t::begin()) {
            if (auto phi = prev(hi); phi->first[1] > R) {
                if (phi->second == color) { // extend new rightwards, removing old
                    R = phi->first[1];
                } else if (phi->first[0] < L) { // splice
                    auto [x, y] = phi->first;
                    auto red = phi->second;
                    processor(intv, red);
                    base_t::erase(phi);
                    base_t::emplace_hint(hi, run_t{x, L}, red);
                    base_t::emplace_hint(hi, run_t{L, R}, color);
                    base_t::emplace_hint(hi, run_t{R, y}, red);
                    return;
                } else { // cut old rightwards and don't remove it
                    if (phi->first[0] < R) {
                        processor(run_t{phi->first[0], R}, phi->second);
                        const_cast<T&>(phi->first[0]) = R;
                    }
                    hi = phi;
                }
            }
        }
        if (lo != base_t::begin()) {
            if (auto plo = prev(lo); plo->first[1] >= L) {
                if (plo->second == color) { // extend new leftwards, removing old
                    L = plo->first[0], lo = plo;
                } else if (plo->first[0] < L) { // cut old leftwards and don't remove it
                    if (plo->first[1] > L) {
                        processor(run_t{L, plo->first[1]}, plo->second);
                        const_cast<T&>(plo->first[1]) = L;
                    }
                } else { // just cut old
                    lo = plo;
                }
            }
        }
        for (auto it = lo; it != hi; ++it) {
            if (it->second != color) {
                processor(it->first, it->second);
            }
        }
        base_t::erase(lo, hi);
        base_t::emplace_hint(hi, intv, color);
    }

    template <typename Fn>
    void cut(run_t intv, Fn&& processor) {
        auto& [L, R] = intv;
        if (L >= R) {
            return;
        }
        auto lo = base_t::lower_bound({L, L});
        auto hi = base_t::lower_bound({R, R});
        if (hi != base_t::begin()) {
            if (auto phi = prev(hi); phi->first[0] < L && phi->first[1] > R) { // splice
                auto [x, y] = phi->first;
                auto color = phi->second;
                processor(intv, color);
                base_t::erase(phi);
                base_t::emplace_hint(hi, run_t{x, L}, color);
                base_t::emplace_hint(hi, run_t{R, y}, color);
                return;
            } else if (phi->first[1] > R) { // cut old rightwards and don't remove it
                if (phi->first[0] < R) {
                    processor(run_t{phi->first[0], R}, phi->second);
                    const_cast<T&>(phi->first[0]) = R;
                }
                hi = phi;
            }
        }
        if (lo != base_t::begin()) {
            if (auto plo = prev(lo); plo->first[1] > L) { // cut old leftwards
                processor(run_t{L, plo->first[1]}, plo->second);
                const_cast<T&>(plo->first[1]) = L;
            }
        }
        for (auto it = lo; it != hi; ++it) {
            processor(it->first, it->second);
        }
        base_t::erase(lo, hi);
    }

    optional<run_t> get_run(T x) const {
        auto it = base_t::upper_bound({x, inf});
        if (it != base_t::begin() && prev(it)->first[0] <= x && x < prev(it)->first[1]) {
            return prev(it)->first;
        }
        return std::nullopt;
    }

    optional<V> get(T x) const {
        auto it = base_t::upper_bound({x, inf});
        if (it != base_t::begin() && prev(it)->first[0] <= x && x < prev(it)->first[1]) {
            return prev(it)->second;
        }
        return std::nullopt;
    }

    V& val(T x) { return prev(base_t::upper_bound({x, inf}))->second; }

    bool contains(T x) const { return get_run(x).has_value(); }

    bool contains(run_t intv) const {
        auto wrap = get_run(intv[0]);
        return wrap.has_value() && wrap->first[1] >= intv[1];
    }

    bool overlaps(run_t intv) const {
        auto lo = base_t::lower_bound({intv[0], intv[0]});
        return !(lo == base_t::end() || lo->first[0] >= intv[1]) ||
               !(lo == base_t::begin() || prev(lo)->first[1] <= intv[0]);
    }

    // Get run including x, or the one closest before x (or end() if none)
    auto before(T x) const {
        auto it = base_t::upper_bound({x, inf});
        return it != base_t::begin() ? prev(it) : base_t::end();
    }

    // Get run strictly before x (or end() if none)
    auto before_strict(T x) const {
        auto it = base_t::upper_bound({x, x});
        if (it != base_t::begin() && prev(it)->first[1] > x)
            --it;
        return it != base_t::begin() ? prev(it) : base_t::end();
    }

    // Get run including x, or the one closest after x (or end() if none)
    auto after(T x) const {
        auto it = base_t::upper_bound({x, inf});
        return it != base_t::begin() && prev(it)->first[1] > x ? prev(it) : it;
    }

    // Get run strictly after x (or end() if none)
    auto after_strict(T x) const {
        return base_t::upper_bound({x, inf}); //
    }
};
