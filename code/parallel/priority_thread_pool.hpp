#pragma once

#include <bits/stdc++.h>
using namespace std;

/**
 * Thread pool designed for a unique owner, jobs processed by priority order.
 * Pretty much identical to the basic thread pool.
 */
template <typename P, typename Compare = less<>>
struct priority_thread_pool {
  private:
    using job_t = function<void()>;
    using entry_t = pair<P, job_t>;
    struct ComparePriority : Compare {
        ComparePriority(const Compare& comp) : Compare(comp) {}
        inline bool operator()(const entry_t& a, const entry_t& b) noexcept {
            return Compare::operator()(a.first, b.first);
        }
    };
    using queue_t = priority_queue<entry_t, vector<entry_t>, ComparePriority>;
    enum pool_status : uint8_t { ready, cancelled, invalid };

    vector<thread> threads;
    queue_t jobs;
    mutable mutex mtx;
    condition_variable cv, cv_user;
    pool_status state = ready;
    unsigned idle = 0, waiting = 0;

    inline int pending_unsafe() const { return jobs.size() + pool_size() - idle; }
    inline int worker_ready() const { return !jobs.empty() || state == cancelled; }

    void init_thread() {
        threads.emplace_back([this]() {
            while (true) {
                unique_lock guard(mtx);
                ++idle;
                if (state == cancelled) {
                    break;
                }
                if (waiting) {
                    cv_user.notify_all();
                }
                if (jobs.empty()) {
                    cv.wait(guard, [this]() { return worker_ready(); });
                }
                if (state == cancelled) {
                    break;
                }
                --idle;
                auto job = move(jobs.top().second);
                jobs.pop();
                guard.unlock();
                job();
            }
        });
    }

  public:
    explicit priority_thread_pool(int nthreads, const Compare& comp = Compare())
        : jobs(comp) {
        assert(nthreads > 0);
        threads.reserve(nthreads);
        for (int id = 0; id < nthreads; id++)
            init_thread();
    }

    ~priority_thread_pool() noexcept { cancel(); }

    /**
     * Submit a job to be run.
     * The function must be callable with the provided arguments.
     * Be careful of argument scope.
     */
    template <typename Fn, typename... Args>
    void submit(P priority, Fn&& fn, Args&&... args) {
        job_t job = bind(forward<Fn>(fn), forward<Args>(args)...);
        lock_guard guard(mtx);
        assert(state == ready);
        jobs.emplace(move(priority), move(job));
        cv.notify_one();
    }

    /**
     * Block until all jobs have been executed.
     * Afterwards the pool is empty and valid, and more jobs can be added.
     */
    void wait() noexcept {
        unique_lock guard(mtx);
        if (pending_unsafe() == 0)
            return;

        waiting++;
        cv_user.wait(guard, [this]() { return pending_unsafe() == 0; });
        waiting--;
    }

    /**
     * Blocks until k more jobs have finished running.
     * Afterwards the pool is valid, and more jobs can be added still.
     */
    void wait_for(int k) {
        unique_lock guard(mtx);
        if (pending_unsafe() == 0)
            return;

        k = max(0, pending_unsafe() - k);
        waiting++;
        cv_user.wait(guard, [this, k]() { return pending_unsafe() <= k; });
        waiting--;
    }

    /**
     * Blocks until there are only k jobs running or pending.
     * If there are already <=k jobs running or pending, wait for 1 job to finish.
     * Afterwards the pool is valid, and more jobs can be added still.
     */
    void wait_until(int k) {
        unique_lock guard(mtx);
        if (pending_unsafe() == 0)
            return;

        k = max(0, min(pending_unsafe() - 1, k));
        waiting++;
        cv_user.wait(guard, [this, k]() { return pending_unsafe() <= k; });
        waiting--;
    }

    /**
     * Block until all jobs have been executed and join with all threads.
     * Afterwards, the pool is empty and invalid.
     */
    void finish() noexcept {
        unique_lock guard(mtx);
        if (state == invalid)
            return;

        waiting++;
        cv_user.wait(guard, [this]() { return pending_unsafe() == 0; });
        waiting--;

        state = cancelled;
        cv.notify_all();
        guard.unlock();
        for (thread& worker : threads)
            assert(worker.joinable()), worker.join();

        state = invalid;
    }

    /**
     * Block until all running jobs have been executed and join with all threads.
     * Jobs pending in the job queue are not executed.
     * Afterwards, the pool is empty and invalid.
     */
    void cancel() {
        unique_lock guard(mtx);
        if (state == invalid)
            return;

        state = cancelled;
        cv.notify_all();
        guard.unlock();
        for (thread& worker : threads)
            assert(worker.joinable()), worker.join();

        state = invalid;
    }

    inline unsigned pending() const noexcept {
        lock_guard guard(mtx);
        return jobs.size() + pool_size() - idle;
    }
    inline unsigned pool_size() const noexcept { return threads.size(); }
    inline bool empty() const noexcept { return pending() == 0; }
};
