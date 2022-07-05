#pragma once

#include "geometry/delaunay.hpp"
#include "geometry/double2d.hpp"

auto circumcenter(Pt2 a, Pt2 b, Pt2 c) {
    double na = norm2(a), nb = norm2(b), nc = norm2(c);
    double D = cross(a, b) + cross(b, c) + cross(c, a);
    double x = (na * (b.y - c.y) + nb * (c.y - a.y) + nc * (a.y - b.y)) / (2 * D);
    double y = (na * (c.x - b.x) + nb * (a.x - c.x) + nc * (b.x - a.x)) / (2 * D);
    return Pd2(x, y);
}

// Computes voronoi diagram by dualizing delaunay and computing circumcenters. O(n log n)
// The point at infinity is vertex 0. The edges in dual are sorted by source vertex
auto voronoi(const vector<Pt2>& pts) {
    auto hull = delaunay(pts);
    detriangulate_delaunay(hull, pts);

    auto [F, dual] = Wedge::dual_all(hull);
    int S = dual.size();

    // Vertex 0 is at infinity
    vector<Pd2> centers(F);
    centers[0] = Pd2(Pd2::inf, Pd2::inf);

    for (int i = 1; i < S; i++) {
        if (int f = dual[i]->vertex; f > dual[i - 1]->vertex) {
            int u = dual[i]->face;
            int v = dual[i]->rotccw()->face;
            int w = dual[i]->rotcw()->face;
            centers[f] = circumcenter(pts[u], pts[v], pts[w]);
        }
    }

    return make_tuple(F, hull, move(dual), move(centers));
}

// Wrap voronoi diagram in a finite squarish bounding box. O(n). Reasonably stable
// The size of the box is noised to avoid the rays hitting the corners exactly
// T must be some edge emanating out from the point at infinity (vertex 0), e.g. dual[0]
auto box_voronoi(const vector<Pt2>& pts, Wedge* T, vector<Pd2>& centers) {
    int N = pts.size(), V = centers.size();
    assert(T->vertex == 0);

    // Put all vertices inside the bounding box, including primal vertices
    Pd2 lo = pts[0], hi = pts[0];
    for (int i = 1; i < N; i++) {
        lo = min(lo, Pd2(pts[i]));
        hi = max(hi, Pd2(pts[i]));
    }
    for (int i = 1; i < V; i++) {
        lo = min(lo, centers[i]);
        hi = max(hi, centers[i]);
    }

    static mt19937 rng(random_device{}());
    static uniform_real_distribution<double> noised(0.007, 0.073);

    // Increase the size of the minimum bounding box by at least width
    double spacing = 1.5 * norm(hi - lo);
    Pd2 offset(spacing, spacing);
    lo -= offset, hi += offset;

    // Add some small perturbation to decrease the odds a ray hits a corner exactly
    Pd2 width = hi - lo;
    auto dx = noised(rng), dy = noised(rng);
    lo.x -= dx * width.x, hi.x += dx * width.x;
    lo.y -= dy * width.y, hi.y += dy * width.y;

    Pd2 a(lo.x, lo.y);
    Pd2 b(lo.x, hi.y); // b > > c
    Pd2 c(hi.x, hi.y); // ^     v
    Pd2 d(hi.x, lo.y); // a < < d
    centers.push_back(a), centers.push_back(b);
    centers.push_back(c), centers.push_back(d);

    // Bounding box lines oriented clockwise
    array<Rayd, 4> sides;
    sides[0] = Rayd::through(a, b);
    sides[1] = Rayd::through(b, c);
    sides[2] = Rayd::through(c, d);
    sides[3] = Rayd::through(d, a);

    // Determine the hitpoint of this edge *backward* with the bounding box
    auto isect_bisector = [&](Wedge* edge, int side) {
        int u = edge->mate->face, v = edge->face;
        Pd2 midpoint = (Pd2(pts[u]) + Pd2(pts[v])) / 2;
        Rayd bisector = Rayd::ray(midpoint, perp_ccw(pts[v] - pts[u]));

        for (int i = 0; i < 4; i++) {
            int j = (side + i) % 4;
            auto hit = isect_unsafe(sides[j], bisector);
            auto bfactor = bisector.coef(hit); // position along bisector
            auto lfactor = sides[j].coef(hit); // position along box side ray

            // Accept if on the positive side of the bisector and within the box segment
            if (bfactor > 0 && 0 <= lfactor && lfactor < 1) {
                return make_tuple(j, hit, lfactor);
            }
        }

        // Well this would be pretty bad, precision error near the corners
        throw runtime_error("Failed to intersect ray with bounding box");
    };

    // The infinite edges spiking into the "hull", pointing out from infinity
    vector<Wedge*> rays;
    do {
        rays.push_back(T);
        T = T->rotccw();
    } while (T != rays[0]);

    // Disconnect the rays from each other to make things easier
    int R = rays.size();
    for (int i = 0; i < R; i++) {
        Wedge::hang_source(rays[i]);
    }

    auto [first_side, first_hit, first_coef] = isect_bisector(T, 0);
    int side = first_side, K = V + 4;
    Wedge* around = T->mate;
    centers[0] = first_hit; // Reuse index 0 as a bounding box hitpoint vertex

    // Stitch the bounding box cw ray by ray, carefully turning 90º around the corners
    for (int j = 1; j < R; j++) {
        T = rays[j];
        auto [next_side, hit, coef] = isect_bisector(T, side);
        while (next_side != side) {
            side = (side + 1) % 4;
            around = Wedge::connecto(around, V + side);
        }
        T->vertex = K++;
        centers.push_back(hit);
        around = Wedge::connect(around, T);
    }

    // Wrap and connect back to the first edge
    while (side != first_side) {
        side = (side + 1) % 4;
        around = Wedge::connecto(around, V + side);
    }
    around = Wedge::connect(around, rays[0]->rotcw());

    // Mark outside and new faces with their respective primal vertices
    Wedge::tag_face(around, -1);
    for (int i = 0; i < R; i++) {
        Wedge::tag_face(rays[i], rays[i]->face);
    }

    return around;
}
