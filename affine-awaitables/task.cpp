//
// demo_affine_task.cpp
//
// Demonstrates task.hpp - a task type built on affine primitives.
// Shows that P3552's task can be implemented using affine_promise and
// affine_task mixins, with support for all awaitable types.
//

#include "task.hpp"
#include "small_function.hpp"
#include "thread_pool.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

//------------------------------------------------------------------------------
// Allocation tracking
//------------------------------------------------------------------------------

std::atomic<size_t> g_allocation_count{0};
std::atomic<bool> g_tracking_enabled{false};

void* operator new(std::size_t size) {
    void* ptr = std::malloc(size);
    if (!ptr)
        throw std::bad_alloc();
    if (g_tracking_enabled.load(std::memory_order_relaxed))
        g_allocation_count.fetch_add(1, std::memory_order_relaxed);
    return ptr;
}

void operator delete(void* ptr) noexcept {
    if (ptr)
        std::free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
    if (ptr)
        std::free(ptr);
}

size_t reset_allocations() {
    size_t count = g_allocation_count.exchange(0);
    g_tracking_enabled.store(true);
    return count;
}

size_t get_allocations() {
    return g_allocation_count.load();
}

void stop_tracking() {
    g_tracking_enabled.store(false);
}

//------------------------------------------------------------------------------
// Simple run loop scheduler
//------------------------------------------------------------------------------

class run_loop {
    std::vector<small_function<void()>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;

public:
    run_loop() {
        queue_.reserve(64);
    }

    template<typename F>
    void dispatch(F&& f) {
        {
            std::lock_guard lock(mutex_);
            queue_.push_back(std::forward<F>(f));
        }
        cv_.notify_one();
    }

    bool run_one() {
        small_function<void()> task;
        {
            std::unique_lock lock(mutex_);
            if (queue_.empty())
                return false;
            task = std::move(queue_.back());
            queue_.pop_back();
        }
        task();
        return true;
    }

    void run() {
        while (run_one()) {}
    }

    void stop() {
        std::lock_guard lock(mutex_);
        stopped_ = true;
        cv_.notify_all();
    }
};

//------------------------------------------------------------------------------
// Type aliases
//------------------------------------------------------------------------------

using my_task = task<void, run_loop>;

template<typename T>
using my_task_t = task<T, run_loop>;

using my_context = task_context<run_loop>;

//------------------------------------------------------------------------------
// Background thread pool for simulating async work
//------------------------------------------------------------------------------

thread_pool* g_pool = nullptr;

//------------------------------------------------------------------------------
// Test awaitables
//------------------------------------------------------------------------------

// Affine awaitable - zero overhead path
template<typename T>
struct affine_async_read {
    T value_;

    bool await_ready() const noexcept { return false; }

    template<typename Dispatcher>
    void await_suspend(std::coroutine_handle<> h, Dispatcher& d) const {
        g_pool->dispatch([h, &d, this]() mutable {
            d(h);
        });
    }

    T await_resume() const noexcept { return value_; }
};

// Legacy awaitable - trampoline path (1 allocation per await)
struct legacy_timer {
    int ms_;

    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> h) const {
        g_pool->dispatch([h, this]() mutable {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms_));
            h.resume();
        });
    }

    void await_resume() const noexcept {}
};

//------------------------------------------------------------------------------
// Test results tracking
//------------------------------------------------------------------------------

struct test_result {
    const char* name;
    size_t allocs;
    bool passed;
};

std::vector<test_result> g_results;

//------------------------------------------------------------------------------
// Test coroutines
//------------------------------------------------------------------------------

// Minimal coroutine - just return immediately
my_task empty_task() {
    co_return;
}

my_task affine_loop_10() {
    for (int i = 0; i < 10; ++i)
        co_await affine_async_read<int>{i};
}

my_task legacy_loop_10() {
    for (int i = 0; i < 10; ++i)
        co_await legacy_timer{1};
}

my_task mixed_2_affine_1_legacy() {
    co_await affine_async_read<int>{1};
    co_await legacy_timer{1};
    co_await affine_async_read<int>{2};
}

my_task_t<int> nested_inner(int x) {
    int a = co_await affine_async_read<int>{x * 2};
    int b = co_await affine_async_read<int>{x * 3};
    co_return a + b;
}

my_task_t<int> nested_outer() {
    int v1 = co_await nested_inner(10);
    int v2 = co_await nested_inner(20);
    co_return v1 + v2;
}

my_task_t<int> may_throw(bool do_throw) {
    co_await affine_async_read<int>{1};
    if (do_throw)
        throw std::runtime_error("intentional error");
    co_return 42;
}

//------------------------------------------------------------------------------
// Helper to run task and count allocations
//------------------------------------------------------------------------------

template<typename T>
size_t run_and_count(task<T, run_loop> t, run_loop& loop) {
    bool done = false;
    t.set_scheduler(loop);
    t.set_done_flag(done);
    
    reset_allocations();
    t.start();
    
    while (!done) {
        loop.run();
    }
    
    stop_tracking();
    return get_allocations();
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

int main() {
    std::cout << "demo_affine_task: P3552-style task with affine primitives\n";
    std::cout << "==========================================================\n";
#if defined(__clang__)
    std::cout << "Compiler: Clang " << __clang_major__ << "." << __clang_minor__ << "\n\n";
#elif defined(_MSC_VER)
    std::cout << "Compiler: MSVC " << _MSC_VER << "\n\n";
#else
    std::cout << "Compiler: Unknown\n\n";
#endif

    thread_pool pool(2);
    g_pool = &pool;
    run_loop loop;

    size_t empty_allocs = 0;
    size_t affine_10 = 0;
    size_t legacy_10 = 0;
    size_t mixed_allocs = 0;
    size_t nested_allocs = 0;

    // Helper to run task and count allocations INCLUDING task creation
    auto run_and_count_full = [&](auto make_task) {
        reset_allocations();
        auto t = make_task();
        bool done = false;
        t.set_scheduler(loop);
        t.set_done_flag(done);
        t.start();
        while (!done) loop.run();
        stop_tracking();
        return get_allocations();
    };

    // Test 0: Empty coroutine (just frame allocation)
    empty_allocs = run_and_count_full([]{ return empty_task(); });

    // Test 1: 10 affine awaits (baseline)
    affine_10 = run_and_count_full([]{ return affine_loop_10(); });

    // Test 2: 10 legacy awaits (+10 trampolines expected)
    legacy_10 = run_and_count_full([]{ return legacy_loop_10(); });

    // Test 3: Mixed (2 affine + 1 legacy)
    mixed_allocs = run_and_count_full([]{ return mixed_2_affine_1_legacy(); });

    // Test 4: Nested tasks (task->task is affine)
    nested_allocs = run_and_count_full([]{ return nested_outer(); });

    // Test 5: Exception propagation
    bool exception_ok = false;
    {
        auto t1 = may_throw(false);
        run_and_count(std::move(t1), loop);
        int result = 0;
        try {
            auto t2 = may_throw(false);
            bool done = false;
            t2.set_scheduler(loop);
            t2.set_done_flag(done);
            t2.start();
            while (!done) loop.run();
            result = t2.handle().promise().result();
        } catch (...) {}

        bool caught = false;
        try {
            auto t3 = may_throw(true);
            bool done = false;
            t3.set_scheduler(loop);
            t3.set_done_flag(done);
            t3.start();
            while (!done) loop.run();
            t3.handle().promise().result();
        } catch (const std::runtime_error&) {
            caught = true;
        }
        exception_ok = (result == 42) && caught;
    }

    // Calculate overhead - legacy should be exactly +10 vs affine baseline
    size_t legacy_overhead = legacy_10 - affine_10;

    // empty_allocs is the pure frame cost (0 with HALO, 1 without)
    bool halo_working = (empty_allocs == 0);
    bool affine_ok = (affine_10 == empty_allocs);  // No extra allocs for affine awaits
    bool legacy_ok = (legacy_overhead == 10);
    // mixed = baseline + 1 trampoline
    bool mixed_ok = (mixed_allocs == empty_allocs + 1);
    // nested = baseline + 2 inner frames
    bool nested_ok = (nested_allocs == empty_allocs + 2);

    g_results.push_back({"HALO (0 = elided, 1 = allocated)", empty_allocs, halo_working});
    g_results.push_back({"10 affine awaits (no overhead)", affine_10, affine_ok});
    g_results.push_back({"10 legacy awaits (+10 trampolines)", legacy_10, legacy_ok});
    g_results.push_back({"2 affine + 1 legacy", mixed_allocs, mixed_ok});
    g_results.push_back({"nested tasks (2 inner frames)", nested_allocs, nested_ok});
    g_results.push_back({"exception propagation", 0, exception_ok});

    // Print results
    std::cout << "Test Results:\n";
    std::cout << "-------------\n";
    std::cout << "  empty coroutine:     " << empty_allocs << " allocs (" 
              << (halo_working ? "HALO!" : "no HALO") << ")\n";
    std::cout << "  10 affine awaits:    " << affine_10 << " allocs\n";
    std::cout << "  10 legacy awaits:    " << legacy_10 << " allocs (+" 
              << legacy_overhead << " trampolines)\n";
    std::cout << "  2 affine + 1 legacy: " << mixed_allocs << " allocs\n";
    std::cout << "  nested tasks:        " << nested_allocs << " allocs\n";
    std::cout << "  exception handling:  " << (exception_ok ? "OK" : "FAILED") << "\n";

    // Print summary checklist
    std::cout << "\nHALO Summary:\n";
    std::cout << "-------------\n";
    bool all_passed = true;
    for (const auto& r : g_results) {
        char mark = r.passed ? '+' : 'X';
        std::cout << "  [" << mark << "] " << r.name << "\n";
        if (!r.passed) all_passed = false;
    }

    std::cout << "\n";
    return all_passed ? 0 : 1;
}
