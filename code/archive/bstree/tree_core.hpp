#pragma once

#include "avl_tree.hpp"
#include "rb_tree.hpp"
#include "splay_tree.hpp"

// Useful to make sure they all compile, at least

template struct avl_tree<int>;
template struct rb_tree<int>;
template struct splay_tree<int>;

template <typename T>
using Tree = rb_tree<T>;
