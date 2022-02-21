#pragma once

#include <bits/stdc++.h>
using namespace std;

// Half edge with no associated face/vertex nodes and 3 data fields (face,mark,data)
struct Wedge {
    struct Pool {
        Wedge* head; // freelist head, next element is head->next
        pmr::monotonic_buffer_resource pool;
        Pool() : head(nullptr) {}
        auto make_edge(int v) {
            if (!head) {
                return new (pool.allocate(sizeof(Wedge))) Wedge(v);
            } else {
                auto E = head;
                head = head->next;
                return new (E) Wedge(v);
            }
        }
        void release() { head = nullptr, pool.release(); }
        void release(Wedge* E) { E->next = head, E->mate->next = E, head = E->mate; }
    };
    static inline Pool primary, temporary;
    static inline Pool* pool = &primary;
    static void release() { primary.release(), temporary.release(); }
    static void use_primary_pool() { pool = &primary, temporary.release(); }
    static void use_temporary_pool() { pool = &temporary; }

    static inline int internal_mark = numeric_limits<int>::min();
    int vertex, face = 0, mark = 0, data = 0;
    Wedge *next = nullptr, *prev = nullptr, *mate = nullptr;
    explicit Wedge(int v) : vertex(v) {}

    inline int target() const { return mate->vertex; }
    inline auto rnext() const { return mate->prev->mate; }
    inline auto rprev() const { return mate->next->mate; }
    inline auto rotccw() const { return prev->mate; }
    inline auto rotcw() const { return mate->next; }

    inline bool hanging_source() const { return prev == mate; }
    inline bool hanging_target() const { return next == mate; }
    inline bool straight_next() const { return next->mate->next == mate; }
    inline bool straight_prev() const { return prev->mate->prev == mate; }
    inline bool hanging() const { return hanging_source() || hanging_target(); }
    inline bool straight() const { return straight_next() || straight_prev(); }

    static void marry(Wedge* A, Wedge* B) { A->mate = B, B->mate = A; }
    static void link(Wedge* A, Wedge* B) { A->next = B, B->prev = A; }
    static void bilink(Wedge* A, Wedge* B) { link(B->mate, A->mate), link(A, B); }

    // --- Constructors

    static auto couple(int a, int b) { // returns a->b
        auto A = pool->make_edge(a);
        marry(A, pool->make_edge(b));
        return A;
    }
    static auto loop(int a, int b) { // returns a->b
        auto A = couple(a, b);
        link(A, A->mate), link(A->mate, A);
        return A;
    }
    static auto line(int a, int b, int c) { // returns a->b
        auto A = couple(a, b);
        auto B = couple(b, c);
        bilink(A, B), link(A->mate, A), link(B, B->mate);
        return A;
    }
    static auto triangle(int a, int b, int c) { // returns a->b
        auto A = couple(a, b);
        auto B = couple(b, c);
        auto C = couple(c, a);
        bilink(A, B), bilink(B, C), bilink(C, A);
        return A;
    }
    static auto tetrahedron(int a, int b, int c, int d) { // return a->b, d above abc
        auto AB = triangle(a, b, c);
        auto DA = couple(d, a);
        auto DB = couple(d, b);
        auto DC = couple(d, c);
        auto BC = AB->next, CA = AB->prev;
        link(DA, AB), link(AB, DB->mate), link(DA->mate, DC);
        link(DB, BC), link(BC, DC->mate), link(DB->mate, DA);
        link(DC, CA), link(CA, DA->mate), link(DC->mate, DB);
        return AB;
    }
    static auto polygon(const vector<int>& vs) { // returns vs[0]->vs[1]
        int N = vs.size();
        if (N == 2) {
            return loop(vs[0], vs[1]);
        }
        Wedge *first = couple(vs[N - 1], vs[0]), *last = first;
        for (int i = 0; i + 1 < N; i++) {
            Wedge* edge = couple(vs[i], vs[i + 1]);
            bilink(last, edge), last = edge;
        }
        bilink(last, first);
        return first->next;
    }

    // --- Connectors

    static auto connect(Wedge* A, Wedge* B) { // A / B => A->E->B, returns E
        auto E = couple(A->target(), B->vertex);
        link(E->mate, A->next), link(A, E);
        link(B->prev, E->mate), link(E, B);
        return E;
    }
    static auto connecto(int v, Wedge* B) { // A->B => E->B and A->E.mate, returns E
        auto E = couple(v, B->vertex);
        link(B->prev, E->mate), link(E, B), link(E->mate, E);
        return E;
    }
    static auto connecto(Wedge* A, int v) { // A->B => A->E and E.mate->B, returns E
        auto E = couple(A->target(), v);
        link(E->mate, A->next), link(A, E), link(E, E->mate);
        return E;
    }
    static auto safe_connect(int u, int v, Wedge* A, Wedge* B) {
        return A ? B ? connect(A, B) : connecto(A, v) : B ? connecto(u, B) : loop(u, v);
    }
    static auto connect_next(Wedge* A) { return connect(A->next, A)->mate; }
    static auto connect_prev(Wedge* B) { return connect(B, B->prev)->mate; }
    static auto connect_rnext(Wedge* A) { return connect(A->mate, A->mate->prev); }
    static auto connect_rprev(Wedge* B) { return connect(B->mate->next, B->mate); }

    // --- Cutters, all checked

    static void cut(Wedge* E) { // {A}->E->{C} => {A} / {C}
        assert(E->next->prev == E && E->prev->next == E);
        link(E->mate->prev, E->next), link(E->prev, E->mate->next);
        pool->release(E);
    }
    static auto cut_ccw(Wedge* E) { // Cut and return E->rotccw() which must be !=E
        auto F = E->rotccw();
        assert(E != F);
        return cut(E), F;
    }
    static auto cut_cw(Wedge* E) { // Cut and return E->rotcw() which must be !=E
        auto F = E->rotcw();
        assert(E != F);
        return cut(E), F;
    }
    static auto meld_with_prev(Wedge* E) { // A->E->C => [A-E]->C, edits A->target
        auto A = E->prev;
        assert(E->mate->next == A->mate && A != E->mate && A->next == E);
        A->mate->vertex = E->target(), link(A, E->next), link(E->mate->prev, A->mate);
        pool->release(E);
        return A;
    }
    static auto meld_with_next(Wedge* E) { // A->E->C => A->[E-C], edits C->vertex
        auto C = E->next;
        assert(E->mate->prev == C->mate && C != E->mate && C->prev == E);
        C->vertex = E->vertex, link(E->prev, C), link(C->mate, E->mate->next);
        pool->release(E);
        return C;
    }
    static auto meld(Wedge* E) { // [{A}->E]->{B} => {A}->{B}, edits A to E->target
        if (E->hanging_source()) {
            auto F = E->next;
            return cut(E), F;
        }
        auto F = E->rotccw();
        while (F != E) {
            F->vertex = E->target(), F = F->rotccw();
        }
        link(E->prev, E->next), link(E->mate->prev, E->mate->next);
        F = E->next, pool->release(E);
        return F;
    }

    // --- Splitters

    static void hang_source(Wedge* E) { link(E->prev, E->mate->next), link(E->mate, E); }
    static void hang_target(Wedge* E) { link(E->mate->prev, E->next), link(E, E->mate); }
    static auto split_before(Wedge* E, int u) { // {A}->E->{B} => {A}->F->E->{B} returns F
        auto F = couple(E->vertex, u);
        link(E->prev, F), link(F->mate, E->mate->next);
        bilink(F, E), E->vertex = u;
        return F;
    }
    static auto split_after(Wedge* E, int u) { // {A}->E->{B} => {A}->E->F->{B} returns F
        auto F = couple(u, E->target());
        link(F, E->next), link(E->mate->prev, F->mate);
        bilink(E, F), E->mate->vertex = u;
        return F;
    }

    // --- Extractors etc

    static auto mark_face(Wedge* F, int mark) {
        Wedge* E = F;
        do {
            F->mark = mark, F = F->next;
        } while (F != E);
    }

    static auto tag_face(Wedge* F, int face) {
        Wedge* E = F;
        do {
            F->face = face, F = F->next;
        } while (F != E);
    }

    static auto linearize(Wedge* T) {
        int visited = internal_mark++;
        vector<Wedge*> bfs;
        auto add_face = [&](Wedge* e) {
            while (e->mark != visited) {
                bfs.push_back(e), e->mark = visited, e = e->next;
            }
        };
        add_face(T);
        for (int i = 0, S = bfs.size(); i < S; i++, S = bfs.size()) {
            add_face(bfs[i]->mate);
        }
        for (int i = 0, S = bfs.size(); i < S; i++) {
            bfs[i]->mark = i;
        }
        return bfs;
    }

    static auto facegroup(Wedge* T) {
        int visited = internal_mark++;
        vector<Wedge*> faces, bfs;
        auto add_face = [&](Wedge* e) {
            if (e->mark != visited) {
                faces.push_back(e);
                do {
                    bfs.push_back(e), e->mark = visited, e = e->next;
                } while (e->mark != visited);
            }
        };
        add_face(T);
        for (int i = 0, S = bfs.size(); i < S; i++, S = bfs.size()) {
            add_face(bfs[i]->mate);
        }
        for (int f = 0, F = faces.size(); f < F; f++) {
            tag_face(faces[f], f);
        }
        return make_pair(move(faces), move(bfs));
    }

    static auto extract_face(Wedge* E, bool canonical = false) {
        vector<int> face;
        Wedge* edge = E;
        do {
            face.push_back(edge->vertex), edge = edge->next;
        } while (edge != E);
        if (canonical) {
            rotate(begin(face), min_element(begin(face), end(face)), end(face));
        }
        return face;
    }

    static auto multi_extract_faces(const vector<Wedge*>& blobs, bool canonical = false) {
        int visited = internal_mark++;
        vector<Wedge*> bfs;
        vector<vector<int>> faces;
        auto add_face = [&](Wedge* e) {
            if (e->mark != visited) {
                vector<int> face;
                do {
                    face.push_back(e->vertex), bfs.push_back(e);
                    e->mark = visited, e = e->next;
                } while (e->mark != visited);
                if (canonical) {
                    rotate(begin(face), min_element(begin(face), end(face)), end(face));
                }
                faces.push_back(move(face));
            }
        };
        for (int i = 0, B = blobs.size(); i < B; i++) {
            if (blobs[i]) {
                add_face(blobs[i]);
            }
        }
        for (int i = 0, S = bfs.size(); i < S; i++, S = bfs.size()) {
            add_face(bfs[i]->mate);
        }
        if (canonical) {
            sort(begin(faces), end(faces));
        }
        return faces;
    }

    static auto extract_faces(Wedge* T, bool canonical = false) {
        return multi_extract_faces({T}, canonical);
    }

    static auto multi_extract_edges(const vector<Wedge*>& blobs, bool directed = false,
                                    bool canonical = false) {
        int visited = internal_mark++;
        vector<Wedge*> bfs;
        vector<array<int, 2>> edges;
        auto add_face = [&](Wedge* e) {
            while (e->mark != visited) {
                if (directed || e->mate->mark == visited) {
                    edges.push_back({e->vertex, e->target()});
                }
                bfs.push_back(e);
                e->mark = visited, e = e->next;
            }
        };
        for (int i = 0, B = blobs.size(); i < B; i++) {
            if (blobs[i]) {
                add_face(blobs[i]);
            }
        }
        for (int i = 0, S = bfs.size(); i < S; i++, S = bfs.size()) {
            add_face(bfs[i]->mate);
        }
        if (canonical && !directed) {
            for (auto& [u, v] : edges) {
                if (u > v) {
                    swap(u, v);
                }
            }
        }
        if (canonical) {
            sort(begin(edges), end(edges));
        }
        return edges;
    }

    static auto extract_edges(Wedge* T, bool directed = false, bool canonical = false) {
        return multi_extract_edges({T}, directed, canonical);
    }

    static auto clone_all(Wedge* T, bool switch_to_primary = false) {
        int visited = internal_mark++;
        vector<Wedge*> bfs;
        auto add_face = [&](Wedge* e) {
            while (e->mark != visited) {
                bfs.push_back(e), e->mark = visited, e = e->next;
            }
        };
        add_face(T);
        int S = bfs.size();
        for (int i = 0; i < S; i++, S = bfs.size()) {
            add_face(bfs[i]->mate);
        }
        for (int i = 0; i < S; i++) {
            bfs[i]->mark = i;
        }
        vector<array<int, 4>> data(S);
        for (int i = 0; i < S; i++) {
            int v = bfs[i]->vertex, f = bfs[i]->face;
            data[i] = {v, f, bfs[i]->mate->mark, bfs[i]->next->mark};
        }
        if (switch_to_primary) {
            use_primary_pool();
        }
        for (int i = 0; i < S; i++) {
            bfs[i] = pool->make_edge(data[i][0]);
            bfs[i]->face = data[i][1];
        }
        for (int i = 0; i < S; i++) {
            bfs[i]->mate = bfs[data[i][2]];
            link(bfs[i], bfs[data[i][3]]);
        }
        return bfs[0];
    }

    static auto dual_all(Wedge* T, bool use_face_labels = false) {
        int visited = internal_mark++, F = 0;
        vector<Wedge*> bfs;
        vector<array<int, 4>> data;
        auto add_face = [&](Wedge* e) {
            if (e->mark != visited) {
                int face = use_face_labels ? e->face : F;
                do {
                    bfs.push_back(e);
                    data.push_back({face, 0, 0, 0});
                    e->mark = visited, e = e->next;
                } while (e->mark != visited);
                F++;
            }
        };
        add_face(T);
        int S = bfs.size();
        for (int i = 0; i < S; i++, S = bfs.size()) {
            add_face(bfs[i]->mate);
        }
        for (int i = 0; i < S; i++) {
            bfs[i]->mark = i;
        }
        for (int i = 0; i < S; i++) {
            int f = data[i][0], v = bfs[i]->target();
            data[i] = {f, v, bfs[i]->mate->mark, bfs[i]->mate->prev->mark};
        }
        for (int i = 0; i < S; i++) {
            bfs[i] = pool->make_edge(data[i][0]);
            bfs[i]->face = data[i][1];
        }
        for (int i = 0; i < S; i++) {
            bfs[i]->mate = bfs[data[i][2]];
            link(bfs[i], bfs[data[i][3]]);
        }
        return make_pair(F, move(bfs));
    }

    // --- Debugging helpers

    friend auto show(Wedge* E) {
        if (E) {
            return '(' + to_string(E->vertex) + ',' + to_string(E->target()) + ')';
        } else {
            return "(NULL)"s;
        }
    }

    friend auto show_mark(Wedge* E) {
        if (E) {
            return '[' + to_string(E->mark) + ';' + to_string(E->mate->mark) + ']';
        } else {
            return "[NULL]"s;
        }
    }

    friend auto show_debug(Wedge* E) {
        if (E) {
            return '{' + show(E->prev) + "~>" + show(E) + "~>" + show(E->next) + '}';
        } else {
            return "{NULL}"s;
        }
    }
};
