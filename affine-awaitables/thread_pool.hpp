//
// thread_pool.hpp
//
// A simple thread pool with pre-allocated storage to avoid
// allocations after construction.
//

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "small_function.hpp"

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include <vector>

/** A simple thread pool with pre-allocated storage.

    The pool uses a fixed number of threads and pre-allocates
    storage for the task queue to avoid allocations after
    construction. It uses small_function to store tasks.
*/
class thread_pool
{
    std::vector<small_function<void()>> queue_;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;

    void
    worker()
    {
        while(true)
        {
            small_function<void()> task;
            {
                std::unique_lock lock(mutex_);
                cv_.wait(lock, [this] {
                    return !queue_.empty() || stopped_;
                });
                if(stopped_ && queue_.empty())
                    return;
                task = std::move(queue_.back());
                queue_.pop_back();
            }
            if(task)
                task();
        }
    }

public:
    explicit
    thread_pool(std::size_t num_threads)
    {
        queue_.reserve(64);
        threads_.reserve(num_threads);
        for(std::size_t i = 0; i < num_threads; ++i)
            threads_.emplace_back([this] { worker(); });
    }

    ~thread_pool()
    {
        {
            std::lock_guard lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
        for(auto& t : threads_)
            t.join();
    }

    thread_pool(thread_pool const&) = delete;
    thread_pool& operator=(thread_pool const&) = delete;

    template<typename F>
    void
    dispatch(F&& f)
    {
        {
            std::lock_guard lock(mutex_);
            queue_.push_back(std::forward<F>(f));
        }
        cv_.notify_one();
    }
};

#endif

