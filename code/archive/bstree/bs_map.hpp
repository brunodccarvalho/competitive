#pragma once

#include "bs_tree.hpp"

template <typename Key, typename Value, typename Compare = std::less<Key>>
struct bs_map : bs_tree<std::pair<const Key, Value>, Compare, map_tag> {
  private:
    using bst = bs_tree<std::pair<const Key, Value>, Compare, map_tag>;
    using T = typename bst::value_type;

    using bst::compare;

  public:
    using key_compare = typename bst::key_compare;
    using key_type = typename bst::key_type;
    using value_type = typename bst::value_type;
    using mapped_type = typename bst::mapped_type;
    using iterator = typename bst::iterator;
    using const_iterator = typename bst::const_iterator;
    using reverse_iterator = typename bst::reverse_iterator;
    using const_reverse_iterator = typename bst::const_reverse_iterator;
    using size_type = typename bst::size_type;
    using difference_type = typename bst::difference_type;
    using reference = typename bst::reference;
    using const_reference = typename bst::const_reference;
    using pointer = typename bst::pointer;
    using const_pointer = typename bst::const_pointer;
    using node_type = typename bst::node_type;
    using insert_return_type = typename bst::insert_return_type;

    using bst::bst;

    template <typename InputIt>
    bs_map(InputIt first, InputIt last) {
        bst::insert_unique(first, last);
    }

    template <typename InputIt>
    bs_map(InputIt first, InputIt last, const Compare& comp) : bst(comp) {
        bst::insert_unique(first, last);
    }

    bs_map(std::initializer_list<value_type> ilist) {
        bst::insert_unique(std::move(ilist));
    }

    bs_map(std::initializer_list<value_type> ilist, const Compare& comp) : bst(comp) {
        bst::insert_unique(std::move(ilist));
    }

    bs_map& operator=(std::initializer_list<value_type> ilist) {
        bst::clear();
        bst::insert_unique(std::move(ilist));
        return *this;
    }

    size_t count(const Key& key) const noexcept { return bst::contains(key); }

    insert_return_type insert(node_type&& nh) {
        return bst::insert_unique(std::move(nh));
    }
    iterator insert(const_iterator hint, node_type&& nh) {
        return bst::insert_hint_unique(hint, std::move(nh));
    }
    std::pair<iterator, bool> insert(const T& data) { return bst::insert_unique(data); }
    iterator insert(const_iterator hint, const T& data) {
        return bst::insert_hint_unique(hint, data);
    }
    std::pair<iterator, bool> insert(T&& data) {
        return bst::insert_unique(std::move(data));
    }
    iterator insert(const_iterator hint, T&& data) {
        return bst::insert_hint_unique(hint, std::move(data));
    }
    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        return bst::insert_unique(first, last);
    }
    void insert(std::initializer_list<T> ilist) {
        return bst::insert_unique(std::move(ilist));
    }
    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return bst::emplace_unique(std::forward<Args>(args)...);
    }
    template <typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args) {
        return bst::emplace_hint_unique(hint, std::forward<Args>(args)...);
    }

    template <typename M>
    std::pair<iterator, bool> insert_or_assign(const Key& key, M&& obj) {
        auto it = bst::lower_bound(key);
        if (it != bst::end() && !compare(key, it->first)) {
            it->second = std::forward<M>(obj);
            return {it, false};
        }
        it = emplace_hint(it, std::piecewise_construct, std::forward_as_tuple(key),
                          std::forward_as_tuple(std::forward<M>(obj)));
        return {it, true};
    }
    template <typename M>
    std::pair<iterator, bool> insert_or_assign(Key&& key, M&& obj) {
        auto it = bst::lower_bound(key);
        if (it != bst::end() && !compare(key, it->first)) {
            it->second = std::forward<M>(obj);
            return {it, false};
        }
        it = emplace_hint(it, std::piecewise_construct,
                          std::forward_as_tuple(std::move(key)),
                          std::forward_as_tuple(std::forward<M>(obj)));
        return {it, true};
    }
    // TODO: hinted insert_or_assign, requires bs_tree support

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const Key& key, Args&&... args) {
        auto it = bst::lower_bound(key);
        if (it != bst::end() && !compare(key, it->first)) {
            return {it, false};
        }
        it = emplace_hint(it, std::piecewise_construct, std::forward_as_tuple(key),
                          std::forward_as_tuple(std::forward<Args>(args)...));
        return {it, true};
    }
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(Key&& key, Args&&... args) {
        auto it = bst::lower_bound(key);
        if (it != bst::end() && !compare(key, it->first)) {
            return {it, false};
        }
        it = emplace_hint(it, std::piecewise_construct,
                          std::forward_as_tuple(std::move(key)),
                          std::forward_as_tuple(std::forward<Args>(args)...));
        return {it, true};
    }
    // TODO: hinted try_emplace, requires bs_tree support

    mapped_type& operator[](const Key& key) {
        auto it = bst::lower_bound(key);
        if (it == bst::end() || compare(key, it->first)) {
            it = emplace_hint(it, std::piecewise_construct, std::forward_as_tuple(key),
                              std::tuple<>());
        }
        return it->second;
    }
    mapped_type& operator[](Key&& key) {
        auto it = bst::lower_bound(key);
        if (it == bst::end() || compare(key, it->first)) {
            it = emplace_hint(it, std::piecewise_construct,
                              std::forward_as_tuple(std::move(key)), std::tuple<>());
        }
        return it->second;
    }
    mapped_type& at(const Key& key) {
        auto it = bst::lower_bound(key);
        if (it == bst::end() || compare(key, it->first))
            throw std::out_of_range("map::at");
        return it->second;
    }
    const mapped_type& at(const Key& key) const {
        auto it = bst::lower_bound(key);
        if (it == bst::end() || compare(key, it->first))
            throw std::out_of_range("map::at");
        return it->second;
    }

    using bst::erase;
    bool erase(const Key& key) { return bst::erase_unique(key); }

    template <typename Compare2>
    void merge(bs_map<Key, Compare2>& src) {
        return bst::merge_unique(src);
    }
    template <typename Compare2>
    void merge(bs_map<Key, Compare2>&& src) {
        return bst::merge_unique(std::move(src));
    }
};

template <typename Key, typename Value, typename Compare = std::less<Key>>
struct bs_multimap : bs_tree<std::pair<const Key, Value>, Compare, map_tag> {
  private:
    using bst = bs_tree<std::pair<const Key, Value>, Compare, map_tag>;
    using T = typename bst::value_type;

    using bst::compare;

  public:
    using key_compare = typename bst::key_compare;
    using key_type = typename bst::key_type;
    using value_type = typename bst::value_type;
    using mapped_type = typename bst::mapped_type;
    using iterator = typename bst::iterator;
    using const_iterator = typename bst::const_iterator;
    using reverse_iterator = typename bst::reverse_iterator;
    using const_reverse_iterator = typename bst::const_reverse_iterator;
    using size_type = typename bst::size_type;
    using difference_type = typename bst::difference_type;
    using reference = typename bst::reference;
    using const_reference = typename bst::const_reference;
    using pointer = typename bst::pointer;
    using const_pointer = typename bst::const_pointer;
    using node_type = typename bst::node_type;
    using insert_return_type = typename bst::insert_return_type;

    using bst::bst;

    template <typename InputIt>
    bs_multimap(InputIt first, InputIt last) {
        bst::insert_multi(first, last);
    }

    template <typename InputIt>
    bs_multimap(InputIt first, InputIt last, const Compare& comp) : bst(comp) {
        bst::insert_multi(first, last);
    }

    bs_multimap(std::initializer_list<value_type> ilist) {
        bst::insert_multi(std::move(ilist));
    }

    bs_multimap(std::initializer_list<value_type> ilist, const Compare& comp)
        : bst(comp) {
        bst::insert_multi(std::move(ilist));
    }

    bs_multimap& operator=(std::initializer_list<value_type> ilist) {
        bst::clear();
        bst::insert_multi(std::move(ilist));
        return *this;
    }

    iterator insert(node_type&& nh) { return bst::insert_multi(std::move(nh)); }
    iterator insert(const_iterator hint, node_type&& nh) {
        return bst::insert_hint_multi(hint, std::move(nh));
    }
    iterator insert(const T& data) { return bst::insert_multi(data); }
    iterator insert(const_iterator hint, const T& data) {
        return bst::insert_hint_multi(hint, data);
    }
    iterator insert(T&& data) { return bst::insert_multi(std::move(data)); }
    iterator insert(const_iterator hint, T&& data) {
        return bst::insert_hint_multi(hint, std::move(data));
    }
    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        return bst::insert_multi(first, last);
    }
    void insert(std::initializer_list<T> ilist) {
        return bst::insert_multi(std::move(ilist));
    }
    template <typename... Args>
    iterator emplace(Args&&... args) {
        return bst::emplace_multi(std::forward<Args>(args)...);
    }
    template <typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args) {
        return bst::emplace_hint_multi(hint, std::forward<Args>(args)...);
    }

    using bst::erase;
    bool erase(const Key& key) { return bst::erase_multi(key); }

    template <typename Compare2>
    void merge(bs_multimap<Key, Compare2>& src) {
        return bst::merge_multi(src);
    }
    template <typename Compare2>
    void merge(bs_multimap<Key, Compare2>&& src) {
        return bst::merge_multi(std::move(src));
    }
};
