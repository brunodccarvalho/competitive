#pragma once

#include "formatting.hpp"
#include "geometry/geometry3d.hpp"
#include "geometry/double3d.hpp"

template <typename Pt>
auto format_obj_vertex(const Pt& pt) {
    return format("v {} {} {}\n", pt[0], pt[1], pt[2]);
}

auto format_obj_face(const vector<int>& face, int offset = 1) {
    string s = "f";
    for (int v : face) {
        s += ' ' + to_string(v + offset);
    }
    return s + '\n';
}

template <typename Pt>
auto format_obj_vertices(const vector<Pt>& pts) {
    string s;
    for (const auto& pt : pts) {
        s += format_obj_vertex(pt);
    }
    return s;
}

auto format_obj_faces(const vector<vector<int>>& faces, int offset = 1) {
    string s;
    for (const auto& face : faces) {
        s += format_obj_face(face, offset);
    }
    return s;
}

auto format_obj_edges(const vector<array<int, 2>>& segments, int offset = 1) {
    string s;
    for (auto [u, v] : segments) {
        s += format("f {} {}\n", u + offset, v + offset);
    }
    return s;
}

template <typename Pt>
auto write_obj_file(const string& file, const vector<Pt>& pts,
                    const vector<vector<int>>& faces, int offset = 1) {
    ofstream out(file);
    assert(out.is_open());
    out << format_obj_vertices(pts) << '\n' << format_obj_faces(faces, offset);
}

template <typename Pt>
auto write_obj_file(const string& file, const vector<Pt>& pts) {
    ofstream out(file);
    assert(out.is_open());
    out << format_obj_vertices(pts) << '\n';
}

template <typename Pt>
auto write_obj_file(const string& file, const vector<Pt>& pts,
                    const vector<array<int, 2>>& segments, int offset = 1) {
    ofstream out(file);
    assert(out.is_open());
    out << format_obj_vertices(pts) << '\n' << format_obj_edges(segments, offset);
}
