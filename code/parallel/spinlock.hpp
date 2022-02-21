#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * A spinlock class that yields after N attempts to lock the spinlock.
 */
template <unsigned N = 16>
class alignas(64) spinlock {
    atomic_flag flag = ATOMIC_FLAG_INIT;

  public:
    spinlock() = default;
    spinlock(const spinlock&) = delete;
    spinlock& operator=(const spinlock&) = delete;

    void lock() noexcept {
        uint16_t count = 0;
        while (flag.test_and_set(std::memory_order_acquire)) {
            if (++count == N) {
                this_thread::yield();
                count = 0;
            }
        }
    }

    void unlock() noexcept { flag.clear(std::memory_order_release); }

    bool try_lock() noexcept {
        uint16_t count = 0;
        while (flag.test_and_set(std::memory_order_acquire)) {
            if (++count == N)
                return false;
        }
        return true;
    }
};

/**
 * A shared mutex based on the spinlock implementation above.
 */
template <unsigned N = 16>
class shared_spinlock {
    spinlock<N> mutex;
    condition_variable_any cv_shared, cv_unique;
    uint16_t state;

    static constexpr uint16_t unique_mask = 1u << 15u;
    static constexpr uint16_t max_readers = uint16_t(~unique_mask);

    bool unique_request() const noexcept { return state & unique_mask; }
    bool unique_mode() const noexcept { return state == unique_mask; }
    uint16_t readers() const noexcept { return state & max_readers; }

  public:
    shared_spinlock() = default;
    shared_spinlock(const shared_spinlock&) = delete;
    shared_spinlock& operator=(const shared_spinlock&) = delete;

    // Writers

    void lock() noexcept {
        unique_lock lock(mutex);
        cv_shared.wait(lock, [&] { return !unique_request(); });
        state |= unique_mask;
        cv_unique.wait(lock, [&] { return !readers(); });
    }

    bool try_lock() {
        unique_lock lock(mutex, std::try_to_lock);
        if (lock.owns_lock() && state == 0) {
            state = unique_mask;
            return true;
        }
        return false;
    }

    void unlock() {
        lock_guard lock(mutex);
        assert(unique_mode());
        state = 0;
        cv_shared.notify_all();
    }

    // Readers

    void lock_shared() {
        unique_lock lock(mutex);
        if (state >= max_readers)
            cv_shared.wait(lock, [&] { return state < max_readers; });
        ++state;
    }

    bool try_lock_shared() {
        unique_lock lock(mutex, std::try_to_lock);
        if (lock.owns_lock() && state < max_readers) {
            ++state;
            return true;
        }
        return false;
    }

    void unlock_shared() {
        lock_guard lock(mutex);
        assert(readers() > 0);
        --state;
        if (unique_mode())
            cv_unique.notify_one();
    }
};

template <typename T>
struct concurrent_queue {
  private:
    vector<T> buf;
    unsigned l = 0, r = 0;
    mutable spinlock<8 * sizeof(T)> mtx;

  public:
    explicit concurrent_queue(unsigned N) : buf(N) {}
    concurrent_queue(const concurrent_queue&) = delete;
    concurrent_queue& operator=(const concurrent_queue&) = delete;

    unsigned size() const {
        lock_guard guard(mtx);
        return r - l;
    }
    bool empty() const {
        lock_guard guard(mtx);
        return l == r;
    }
    void reset() { l = r = 0; }

    void push(const T& val) {
        lock_guard guard(mtx);
        assert(r < buf.size());
        buf[r++] = val;
    }
    const T& pop() {
        lock_guard guard(mtx);
        assert(l < r);
        return buf[l++];
    }
    bool maybe_pop(T& elem) {
        lock_guard guard(mtx);
        return l < r ? elem = buf[l++], true : false;
    }
};
