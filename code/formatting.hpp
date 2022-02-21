#pragma once

#include <bits/stdc++.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace std;
using fmt::print;
using fmt::format;

string repeat(int n, const string& pat) {
    string s;
    for (int i = 0; i < n; i++)
        s += pat;
    return s;
}
string repeat(int n, const string& pat, const string& sep) {
    string s;
    for (int i = 0; i < n; i++)
        s += (i ? sep : "") + pat;
    return s;
}

string make_format_string(int n) { return repeat(n, "{}", " "); }

template <typename... Ts>
auto putln(Ts&&... args) {
    return print(make_format_string(sizeof...(Ts)) + "\n", std::forward<Ts>(args)...);
}

template <typename... Ts>
auto putln(ostream& out, Ts&&... args) {
    return print(out, make_format_string(sizeof...(Ts)) + "\n",
                 std::forward<Ts>(args)...);
}

template <typename... Ts>
auto eputln(Ts&&... args) {
    return putln(cerr, std::forward<Ts>(args)...);
}

template <typename... Ts>
auto println(string fmt, Ts&&... args) {
    return print(fmt + "\n", std::forward<Ts>(args)...);
}

template <typename... Ts>
auto println(ostream& out, string fmt, Ts&&... args) {
    return print(out, fmt + "\n", std::forward<Ts>(args)...);
}

template <typename... Ts>
auto eprintln(Ts&&... args) {
    return println(cerr, std::forward<Ts>(args)...);
}

template <typename... Ts>
auto eprint(Ts&&... args) {
    return print(cerr, std::forward<Ts>(args)...);
}

template <typename Seq>
string seq_to_string(const Seq& v) {
    string s;
    for (const auto& el : v)
        s += to_string(el) + " ";
    return s.empty() ? s : (s.pop_back(), s);
}

void pad_strings_in_matrix(vector<vector<string>>& mat, bool pad_left = true) {
    vector<size_t> width;
    for (int i = 0, N = mat.size(); i < N; i++) {
        width.resize(max(width.size(), mat[i].size()));
        for (int j = 0, M = mat[i].size(); j < M; j++) {
            width[j] = max(width[j], mat[i][j].size());
        }
    }
    for (int i = 0, N = mat.size(); i < N; i++) {
        for (int j = 0, M = mat[i].size(); j < M; j++) {
            string pad(width[j] - mat[i][j].size(), ' ');
            if (pad_left) {
                mat[i][j] = pad + mat[i][j];

            } else {
                mat[i][j] = mat[i][j] + pad;
            }
        }
    }
}

string build_aligned_string(const vector<vector<string>>& mat) {
    vector<size_t> width;
    for (int i = 0, N = mat.size(); i < N; i++) {
        width.resize(max(width.size(), mat[i].size()));
        for (int j = 0, M = mat[i].size(); j < M; j++) {
            width[j] = max(width[j], mat[i][j].size());
        }
    }
    string s;
    for (int i = 0, N = mat.size(); i < N; i++) {
        for (int j = 0, M = mat[i].size(); j < M; j++) {
            string pad(width[j] - mat[i][j].size(), ' ');
            s += pad + mat[i][j] + " \n"[j + 1 == M];
        }
    }
    return s;
}

template <typename Mat>
string mat_to_string(const Mat& v) {
    vector<vector<string>> string_matrix;
    for (const auto& row : v) {
        string_matrix.emplace_back();
        for (const auto& col : row) {
            if constexpr (std::is_same<decltype(col), const string&>::value) {
                string_matrix.back().push_back(col);
            } else {
                using std::to_string;
                string_matrix.back().push_back(to_string(col));
            }
        }
    }
    return build_aligned_string(string_matrix);
}

template <typename Mat>
string mat_to_string_indices(const Mat& v) {
    vector<vector<string>> string_matrix(1, vector<string>(1, ""));
    int M = 0;
    for (const auto& row : v) {
        string_matrix.emplace_back();
        string_matrix.back().push_back(std::to_string(string_matrix.size() - 2));
        for (const auto& col : row) {
            if constexpr (std::is_same<decltype(col), const string&>::value) {
                string_matrix.back().push_back(col);
            } else {
                using std::to_string;
                string_matrix.back().push_back(to_string(col));
            }
        }
        M = max(M, int(string_matrix.back().size()));
    }
    for (int i = 1; i < M; i++) {
        string_matrix[0].push_back(std::to_string(i - 1));
    }
    return build_aligned_string(string_matrix);
}

template <typename U, typename V, typename String>
string format_pair_map(const map<pair<U, V>, String>& times) {
    if (times.empty()) {
        return "";
    }
    set<U> rowset;
    set<V> colset;
    for (const auto& [key, time] : times) {
        rowset.insert(key.first);
        colset.insert(key.second);
    }
    vector<U> rows(begin(rowset), end(rowset));
    vector<V> cols(begin(colset), end(colset));
    int R = rows.size(), C = cols.size();
    vector<vector<string>> string_matrix(R + 1, vector<string>(C + 1));
    for (int r = 0; r < R; r++) {
        for (int c = 0; c < C; c++) {
            if (times.count({rows[r], cols[c]})) {
                if constexpr (is_same<String, string>::value) {
                    string_matrix[r + 1][c + 1] = times.at({rows[r], cols[c]});
                } else {
                    string_matrix[r + 1][c + 1] = to_string(times.at({rows[r], cols[c]}));
                }
            }
        }
    }
    for (int r = 0; r < R; r++) {
        using std::to_string;
        if constexpr (is_same<U, string>::value) {
            string_matrix[r + 1][0] = rows[r];
        } else {
            string_matrix[r + 1][0] = to_string(rows[r]);
        }
    }
    for (int c = 0; c < C; c++) {
        using std::to_string;
        if constexpr (is_same<V, string>::value) {
            string_matrix[0][c + 1] = cols[c];
        } else {
            string_matrix[0][c + 1] = to_string(cols[c]);
        }
    }
    return build_aligned_string(string_matrix);
}

template <typename U, typename V, typename W, typename String>
string format_tuple_map(const map<tuple<U, V, W>, String>& times, bool label_row = 0) {
    if (times.empty()) {
        return "";
    }
    set<U> rowset;
    set<V> colset;
    set<W> valset;
    for (const auto& [key, time] : times) {
        rowset.insert(std::get<0>(key));
        colset.insert(std::get<1>(key));
        valset.insert(std::get<2>(key));
    }
    vector<U> rows(begin(rowset), end(rowset));
    vector<V> cols(begin(colset), end(colset));
    vector<W> vals(begin(valset), end(valset));
    int R = rows.size(), C = cols.size(), N = vals.size();
    vector<vector<string>> string_matrix(R + 1 + label_row, vector<string>(C + 1));
    vector<string> val_labels;
    if (label_row) {
        for (int w = 0; w < N; w++) {
            if constexpr (is_same<W, string>::value) {
                val_labels.push_back(vals[w]);
            } else {
                val_labels.push_back(to_string(vals[w]));
            }
        }
    }
    for (int c = 0; c < C; c++) {
        vector<vector<string>> mat(R, vector<string>(N));
        for (int r = 0; r < R; r++) {
            for (int w = 0; w < N; w++) {
                if (times.count({rows[r], cols[c], vals[w]})) {
                    if constexpr (is_same<String, string>::value) {
                        mat[r][w] = times.at({rows[r], cols[c], vals[w]});
                    } else {
                        mat[r][w] = to_string(times.at({rows[r], cols[c], vals[w]}));
                    }
                }
            }
        }
        vector<size_t> width(N);
        for (int r = 0; r < R; r++) {
            for (int w = 0; w < N; w++) {
                width[w] = max(width[w], mat[r][w].size());
            }
        }
        if (label_row) {
            for (int w = 0; w < N; w++) {
                width[w] = max(width[w], val_labels[w].size());
            }
            string_matrix[1][c + 1] = "|";
            for (int w = 0; w < N; w++) {
                string pad(width[w] - val_labels[w].size(), ' ');
                string_matrix[1][c + 1] += " " + pad + val_labels[w];
            }
        }
        for (int r = 0; r < R; r++) {
            string_matrix[r + 1 + label_row][c + 1] = "|";
            for (int w = 0; w < N; w++) {
                string pad(width[w] - mat[r][w].size(), ' ');
                string_matrix[r + 1 + label_row][c + 1] += " " + pad + mat[r][w];
            }
        }
    }
    for (int r = 0; r < R; r++) {
        using std::to_string;
        if constexpr (is_same<U, string>::value) {
            string_matrix[r + 1 + label_row][0] = rows[r];
        } else {
            string_matrix[r + 1 + label_row][0] = to_string(rows[r]);
        }
    }
    for (int c = 0; c < C; c++) {
        using std::to_string;
        if constexpr (is_same<V, string>::value) {
            string_matrix[0][c + 1] = cols[c];
        } else {
            string_matrix[0][c + 1] = to_string(cols[c]);
        }
    }
    return build_aligned_string(string_matrix);
}

template <typename U>
string format_histogram(const map<U, int>& hist) {
    long sum = 0;
    int max_frequency = 0;
    for (const auto& [key, frequency] : hist) {
        sum += frequency;
        max_frequency = max(max_frequency, frequency);
    }
    assert(max_frequency > 0);
    constexpr int BARS = 60;
    vector<vector<string>> table;
    for (const auto& [key, frequency] : hist) {
        int length = llround(1.0 * frequency / max_frequency * BARS);
        string bars = string(length, '#') + string(BARS - length, ' ');
        table.push_back({to_string(key), to_string(frequency), bars});
    }
    return build_aligned_string(table);
}

template <typename T>
map<T, int> make_histogram(const vector<T>& occurrences) {
    map<T, int> hist;
    for (T n : occurrences) {
        hist[n]++;
    }
    return hist;
}

template <typename T>
map<T, int> make_amortized_histogram(const vector<T>& occurrences, int rows) {
    T tmin = *min_element(begin(occurrences), end(occurrences));
    T tmax = *max_element(begin(occurrences), end(occurrences));
    T block = (tmax - tmin + rows - 1) / rows;
    block = block > 0 ? block : 1;
    map<T, int> hist;
    for (long n : occurrences) {
        T b = (n - tmin) / block;
        hist[tmin + block * b]++;
    }
    return hist;
}

template <typename... Ts>
void debugger(string_view vars, Ts&&... args) {
    cout.flush(), cerr.flush();
    cerr << ">> [" << vars << "]: ";
    const char* delim = "";
    (..., (cerr << delim << args, delim = ", "));
    cerr << endl;
}
#define debug(...) debugger(#__VA_ARGS__, __VA_ARGS__)

namespace std {

const string& to_string(const string& s) { return s; }

template <typename U, typename V>
string to_string(const pair<U, V>& uv) {
    return '(' + to_string(uv.first) + ',' + to_string(uv.second) + ')';
}
template <typename U, typename V>
ostream& operator<<(ostream& out, const pair<U, V>& v) {
    return out << to_string(v);
}

template <typename T>
string to_string(const array<T, 2>& uv) {
    return '(' + to_string(uv[0]) + ',' + to_string(uv[1]) + ')';
}
template <typename T, size_t N>
ostream& operator<<(ostream& out, const array<T, N>& v) {
    return out << to_string(v);
}

template <typename T, typename... Rs>
string to_string(const vector<T, Rs...>& v) {
    return seq_to_string(v);
}
template <typename T, typename... Rs>
ostream& operator<<(ostream& out, const vector<T, Rs...>& v) {
    return out << to_string(v);
}

template <typename T, typename... Rs>
string to_string(const vector<vector<T, Rs...>>& v) {
    return mat_to_string(v);
}
template <typename T, typename... Rs>
ostream& operator<<(ostream& out, const vector<vector<T, Rs...>>& v) {
    return out << to_string(v);
}

template <typename K, typename V, typename... Rs>
string to_string(const map<K, V, Rs...>& v) {
    return seq_to_string(v);
}
template <typename K, typename V, typename... Rs>
ostream& operator<<(ostream& out, const map<K, V, Rs...>& v) {
    return out << to_string(v);
}

template <typename T, typename... Rs>
string to_string(const deque<T, Rs...>& v) {
    return seq_to_string(v);
}
template <typename T, typename... Rs>
ostream& operator<<(ostream& out, const deque<T, Rs...>& v) {
    return out << to_string(v);
}

template <typename T, size_t N>
string to_string(const array<T, N>& v) {
    return seq_to_string(v);
}

template <typename T, typename... Rs>
string to_string(const list<T, Rs...>& v) {
    return seq_to_string(v);
}
template <typename T, typename... Rs>
ostream& operator<<(ostream& out, const list<T, Rs...>& v) {
    return out << to_string(v);
}

template <typename T, typename... Rs>
string to_string(const set<T, Rs...>& v) {
    return seq_to_string(v);
}
template <typename T, typename... Rs>
ostream& operator<<(ostream& out, const set<T, Rs...>& v) {
    return out << to_string(v);
}

template <typename T, typename... Rs>
string to_string(const unordered_set<T, Rs...>& v) {
    return seq_to_string(v);
}
template <typename T, typename... Rs>
ostream& operator<<(ostream& out, const unordered_set<T, Rs...>& v) {
    return out << to_string(v);
}

template <typename T, typename... Rs>
string to_string(const multiset<T, Rs...>& v) {
    return seq_to_string(v);
}
template <typename T, typename... Rs>
ostream& operator<<(ostream& out, const multiset<T, Rs...>& v) {
    return out << to_string(v);
}

template <typename T, typename... Rs>
string to_string(const unordered_multiset<T, Rs...>& v) {
    return seq_to_string(v);
}
template <typename T, typename... Rs>
ostream& operator<<(ostream& out, const unordered_multiset<T, Rs...>& v) {
    return out << to_string(v);
}

template <typename K, typename V, typename... Rs>
string to_string(const unordered_map<K, V, Rs...>& v) {
    return seq_to_string(v);
}
template <typename K, typename V, typename... Rs>
ostream& operator<<(ostream& out, const unordered_map<K, V, Rs...>& v) {
    return out << to_string(v);
}

template <typename K, typename V, typename... Rs>
string to_string(const multimap<K, V, Rs...>& v) {
    return seq_to_string(v);
}
template <typename K, typename V, typename... Rs>
ostream& operator<<(ostream& out, const multimap<K, V, Rs...>& v) {
    return out << to_string(v);
}

template <typename K, typename V, typename... Rs>
string to_string(const unordered_multimap<K, V, Rs...>& v) {
    return seq_to_string(v);
}
template <typename K, typename V, typename... Rs>
ostream& operator<<(ostream& out, const unordered_multimap<K, V, Rs...>& v) {
    return out << to_string(v);
}

template <std::size_t N, std::size_t... I>
struct gen_indices : gen_indices<(N - 1), (N - 1), I...> {};
template <std::size_t... I>
struct gen_indices<0, I...> : integer_sequence<std::size_t, I...> {};

string to_string(char n) { return string(1, n); }

template <typename H>
std::string& to_string_impl(std::string& s, H&& h) {
    using std::to_string;
    s += to_string(std::forward<H>(h));
    s += ' ';
    return s;
}

template <typename H, typename... T>
std::string& to_string_impl(std::string& s, H&& h, T&&... t) {
    using std::to_string;
    s += to_string(std::forward<H>(h));
    s += ' ';
    return to_string_impl(s, std::forward<T>(t)...);
}

template <typename... T, std::size_t... I>
std::string to_string(const std::tuple<T...>& tup, integer_sequence<std::size_t, I...>) {
    std::string result;
    int ctx[] = {(to_string_impl(result, std::get<I>(tup)...), 0), 0};
    (void)ctx;
    return result.empty() ? "()" : "(" + (result.pop_back(), result) + ")";
}

template <typename... T>
std::string to_string(const std::tuple<T...>& tup) {
    return to_string(tup, gen_indices<sizeof...(T)>{});
}

} // namespace std

template <typename T>
string to_string(const T uv[2]) {
    return '(' + to_string(uv[0]) + ',' + to_string(uv[1]) + ')';
}
template <typename T, size_t N>
ostream& operator<<(ostream& out, const T v[2]) {
    return out << to_string(v);
}

struct stringable {
    string txt;

    stringable() = default;
    template <typename T>
    stringable(T&& arg) : txt(to_string(arg)) {}
    stringable(const char* arg) : txt(arg) {}
    stringable(string&& arg) : txt(move(arg)) {}
    stringable(const string& arg) : txt(arg) {}
    stringable(string& arg) : txt(arg) {}
    stringable(char c) : txt(1, c) {}

    friend const string& to_string(const stringable& s) { return s.txt; }
    operator string const &() const { return txt; }

    bool operator<(const stringable& b) const { return txt < b.txt; }
    bool operator==(const stringable& b) const { return txt == b.txt; }
};
