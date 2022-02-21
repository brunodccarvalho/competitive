#pragma once

#include <bits/stdc++.h>
using namespace std;

// Order statistics tree/map

#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
namespace gnu = __gnu_pbds;

template <typename Key, typename Value, typename CompareFn = less<Key>>
using ordered_map = gnu::tree<Key, Value, CompareFn, gnu::rb_tree_tag,
                              gnu::tree_order_statistics_node_update>;

template <typename T, typename CompareFn = less<T>>
using ordered_set = ordered_map<T, gnu::null_type, CompareFn>;

// Fast min priority queue

#include <ext/pb_ds/priority_queue.hpp>
namespace gnu = __gnu_pbds;

template <typename T, typename CompareFn = less<T>>
using max_heap = gnu::priority_queue<T, CompareFn, gnu::thin_heap_tag>;
// using min_heap_t = max_heap<pair<long, int>, greater<>>;
// using iter_t = heap_t::point_const_iterator;

// Fast hashset/map

#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/hash_policy.hpp>
namespace gnu = __gnu_pbds;

template <typename K, typename V, typename Hash = std::hash<K>>
using hash_map = gnu::gp_hash_table<K, V, Hash>;

template <typename K, typename Hash = std::hash<K>>
using hash_set = hash_map<K, gnu::null_type, Hash>;

namespace __gnu_pbds {

template <typename T>
string to_string(const ordered_set<T>& os) {
    return seq_to_string(os);
}

template <typename T>
ostream& operator<<(ostream& out, const ordered_set<T>& os) {
    return out << to_string(os);
}

} // namespace __gnu_pbds
