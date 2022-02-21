#pragma once

#include "geometry/geometry2d.hpp"

// Given a sweepline-sorted ranked list of points and non-crossing segments pointing along
// the sweep direction, determine an order of the segments along a CCW perpendicular to
// this sweep direction. Given a point P, determine the segment lying to its right.
// For a lexicographic sweep along forward=P(1,0), the segments are sorted from +y to -y.
struct shaft_scanner {
    using is_transparent = bool;

    const vector<Pt2>& pts;
    const vector<int>& rank;
    const vector<array<int, 2>>& segments;

    shaft_scanner(const vector<Pt2>& pts, const vector<int>& rank,
                  const vector<array<int, 2>>& segments)
        : pts(pts), rank(rank), segments(segments) {}

    bool operator()(int i, int j) const {
        auto [a, b] = segments[i];
        auto [c, d] = segments[j];
        if (i == j) {
            return false;
        } else if (a == c) {
            return cross(pts[a], pts[d], pts[b]) > 0;
        } else if (b == d) {
            return cross(pts[b], pts[a], pts[c]) > 0;
        } else if (rank[a] < rank[c]) {
            if (auto acb = cross(pts[a], pts[c], pts[b])) {
                return acb > 0;
            } // fall through if a,c,b are collinear
        } else /* rank[a] > rank[c] */ {
            if (auto cda = cross(pts[c], pts[d], pts[a])) {
                return cda > 0;
            } // fall through if c,a,d are collinear
        }
        return cross(pts[b] - pts[a], pts[d] - pts[c]) < 0;
    }

    bool operator()(const Pt2& p, int i) const {
        auto [a, b] = segments[i];
        if (auto cvp = cross(pts[b] - pts[a], p - pts[a])) {
            return cvp > 0;
        } else {
            return dot(p - pts[a], pts[b] - pts[a]) < 0;
        }
    }

    bool operator()(int i, const Pt2& p) const {
        auto [a, b] = segments[i];
        if (auto cpu = cross(p - pts[a], pts[b] - pts[a])) {
            return cpu > 0;
        } else {
            return dot(p - pts[b], pts[a] - pts[b]) < 0;
        }
    }
};

// Performing sweepline along a forward direction, sort segments along the CW of this
// direction. Segments must be oriented and must not cross or overlap (may touch)
// If forward=P(1,0), the segments are sorted from +y to -y.
// "Horizontal" segments can be oriented either way (but consistently)
struct live_sweep_scanner {
    using is_transparent = bool;

    Pt2 forward;

    explicit live_sweep_scanner(Pt2 forward) : forward(forward) { assert(forward); }

    bool operator()(const Ray& u, const Ray& v) const {
        if (auto duv = dot(u.p - v.p, forward); duv == 0) {
            if (auto cuv = cross(v.p - u.p, forward)) {
                return cuv > 0;
            }
        } else if (duv < 0) {
            if (auto cuv = cross(v.p - u.p, u.d)) {
                return cuv > 0;
            } // falls through if v.p lies on u
        } else if (duv > 0) {
            if (auto cuv = cross(v.d, u.p - v.p)) {
                return cuv > 0;
            } // falls through if u.p lies on v
        }
        return cross(u.d, v.d) < 0;
    }

    bool operator()(const Pt2& p, const Ray& v) const {
        if (auto cvp = cross(v.d, p - v.p)) {
            return cvp > 0;
        } else {
            return cross(v.p - p, forward) > 0 && cross(v.q() - p, forward) > 0;
        }
    }

    bool operator()(const Ray& u, const Pt2& p) const {
        if (auto cpu = cross(p - u.p, u.d)) {
            return cpu > 0;
        } else {
            return cross(forward, p - u.p) > 0 && cross(forward, p - u.q()) > 0;
        }
    }

    bool sweep_compare(const Pt2& a, const Pt2& b) const { // rank[a] <=> rank[b]
        if (auto ab = dot(b - a, forward)) {
            return ab > 0;
        } else {
            return cross(b - a, forward) > 0; // left to right along CW(forward)
            // return cross(a - b, forward) > 0; // right to left along CW(forward)
        }
    }

    // Return true if the ray u is correctly oriented, and sweep hits u.p before u.q
    bool oriented(const Ray& u) const { return sweep_compare(u.p, u.q()); }

    bool group(const Pt2& u, const Pt2& v) const { return dot(u - v, forward) == 0; }
};

// Sort segments radially "away" from pivot, closest to furthest. Segments must be
// oriented, not cross or overlap (may touch), and must not cross the origin (may touch)
// Segments collinear with the pivot can be oriented either way (but consistently)
struct live_angle_scanner {
    using is_transparent = bool;

    Pt2 pivot;

    explicit live_angle_scanner(Pt2 pivot = Pt2()) : pivot(pivot) {}

    bool operator()(const Ray& u, const Ray& v) const {
        if (auto sign = cross(pivot, v.p, u.p); sign == 0) {
            if (auto nuv = dist2(v.p, pivot) - dist2(u.p, pivot)) {
                return nuv > 0;
            }
        } else if (sign < 0) {
            if (auto cuv = cross(v.p - u.p, u.d)) {
                return cuv > 0;
            }
        } else if (sign > 0) {
            if (auto cuv = cross(v.d, u.p - v.p)) {
                return cuv > 0;
            }
        }
        return cross(u.d, v.d) < 0;
    }

    bool operator()(const Pt2& p, const Ray& v) const {
        if (auto cvp = cross(v.d, p - v.p)) {
            return cvp > 0;
        } else {
            return dot(p - pivot, v.p - p) > 0 && dot(p - pivot, v.q() - p) > 0;
        }
    }

    bool operator()(const Ray& u, const Pt2& p) const {
        if (auto cpu = cross(p - u.p, u.d)) {
            return cpu > 0;
        } else {
            return dot(p - pivot, p - u.p) > 0 && dot(p - pivot, p - u.q()) > 0;
        }
    }

    // Return true if the sweep travels less from u to v than from v to u
    bool sweep_shorter(const Pt2& a, const Pt2& b) const {
        if (auto ab = cross(pivot, a, b)) {
            return ab > 0;
        } else {
            return dot(a - pivot, b - a) > 0; // facing away from pivot
            // return dot(a - pivot, a - b) > 0; // facing into pivot
        }
    }

    // Return true if the ray u is correctly oriented
    bool oriented(const Ray& ray) const { return sweep_shorter(ray.p, ray.q()); }

    bool group(const Pt2& u, const Pt2& v) const { return cross(pivot, u, v) == 0; }
};
