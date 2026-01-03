//
// demo_my_task.cpp
//
// Demonstrates how to build a custom task type using the
// affine_promise and affine_task mixins.
//

#include "affine.hpp"
#include "affine_helpers.hpp"
#include "make_affine.hpp"
#include "small_function.hpp"
#include "thread_pool.hpp"

#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <mutex>
#include <optional>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

//------------------------------------------------------------------------------
// Allocation tracking
//------------------------------------------------------------------------------

std::atomic<size_t> g_allocation_count{0};
std::atomic<bool> g_tracking_enabled{false};

void*
operator new(std::size_t size)
{
    void* ptr = std::malloc(size);
    if (!ptr)
        throw std::bad_alloc();
    if (g_tracking_enabled.load(std::memory_order_relaxed))
        g_allocation_count.fetch_add(1, std::memory_order_relaxed);
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    if (ptr)
        std::free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept
{
    if (ptr)
        std::free(ptr);
}

size_t reset_allocations()
{
    size_t old = g_allocation_count.exchange(0);
    g_tracking_enabled.store(true);
    return old;
}

size_t get_allocations()
{
    return g_allocation_count.load();
}

void stop_tracking()
{
    g_tracking_enabled.store(false);
}

//------------------------------------------------------------------------------
// Simple executor
//------------------------------------------------------------------------------

class simple_executor
{
    std::string_view name_;
    std::vector<small_function<void()>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;

public:
    simple_executor(std::string_view name) : name_(name) {}

    void reserve(std::size_t n) { queue_.reserve(n); }

    template<typename F>
    void dispatch(F&& f)
    {
        std::lock_guard lock(mutex_);
        queue_.push_back(std::forward<F>(f));
        cv_.notify_one();
    }

    void run()
    {
        while (true) {
            small_function<void()> task;
            {
                std::unique_lock lock(mutex_);
                cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
                if (stopped_ && queue_.empty())
                    return;
                task = std::move(queue_.back());
                queue_.pop_back();
            }
            task();
        }
    }

    void stop()
    {
        std::lock_guard lock(mutex_);
        stopped_ = true;
        cv_.notify_all();
    }

    std::string_view name() const { return name_; }
};

using executor_context = resume_context<simple_executor>;

//------------------------------------------------------------------------------
// my_task: A custom task type using affine mixins
//------------------------------------------------------------------------------

template<typename T = void>
class my_task;

template<>
class my_task<void>
    : public affine_task<void, my_task<void>, executor_context>
{
public:
    struct promise_type
        : affine_promise<promise_type, executor_context>
    {
        std::exception_ptr exception_;

        my_task get_return_object()
        {
            return my_task{handle_type::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept { return {}; }

        void return_void() noexcept {}

        void unhandled_exception()
        {
            exception_ = std::current_exception();
        }

        void result()
        {
            if (exception_)
                std::rethrow_exception(exception_);
        }

        template<typename Awaitable>
        auto await_transform(Awaitable&& a)
        {
            using A = std::remove_cvref_t<Awaitable>;

            if constexpr (affine_awaitable<A, executor_context>) {
                return affine_awaiter{
                    std::forward<Awaitable>(a), &*this->dispatcher_};
            } else {
                return make_affine(
                    std::forward<Awaitable>(a), *this->dispatcher_);
            }
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

private:
    handle_type handle_;

public:
    explicit my_task(handle_type h) : handle_(h) {}
    my_task(my_task&& o) noexcept : handle_(std::exchange(o.handle_, {})) {}
    ~my_task() { if (handle_) handle_.destroy(); }

    handle_type handle() const { return handle_; }

    void set_executor(simple_executor& ex)
    {
        handle_.promise().set_dispatcher(executor_context{ex});
    }

    void set_done_flag(bool& flag)
    {
        handle_.promise().set_done_flag(flag);
    }

    void start() { handle_.resume(); }
};

//------------------------------------------------------------------------------
// Example awaitables
//------------------------------------------------------------------------------

thread_pool* g_pool = nullptr;

// Legacy awaitable - resumes directly, no affinity support
struct legacy_async_op
{
    int result_;

    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> h) const
    {
        g_pool->dispatch([h]() mutable { h.resume(); });
    }

    int await_resume() const noexcept { return result_; }
};

// Affine awaitable - supports dispatcher for zero-alloc affinity
struct affine_async_op
{
    int result_;

    bool await_ready() const noexcept { return false; }

    template<typename Dispatcher>
    void await_suspend(std::coroutine_handle<> h, Dispatcher& d) const
    {
        g_pool->dispatch([h, &d]() mutable {
            d(h);
        });
    }

    int await_resume() const noexcept { return result_; }
};

// Simple yield that resumes immediately
struct yield_awaitable
{
    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> h) const
    {
        h.resume();
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

int g_result = 0;

// Minimal coroutine - just return immediately
my_task<void> empty_task()
{
    co_return;
}

my_task<void> test_affine_only()
{
    int x = co_await affine_async_op{10};
    int y = co_await affine_async_op{20};
    int z = co_await affine_async_op{30};
    g_result = x + y + z;
}

my_task<void> test_legacy_only()
{
    int x = co_await legacy_async_op{10};
    int y = co_await legacy_async_op{20};
    int z = co_await legacy_async_op{30};
    g_result = x + y + z;
}

my_task<void> test_mixed()
{
    int x = co_await affine_async_op{10};
    co_await yield_awaitable{};  // legacy
    int y = co_await affine_async_op{20};
    g_result = x + y;
}


//------------------------------------------------------------------------------
// Helper to run task and count allocations INCLUDING task creation
//------------------------------------------------------------------------------

size_t run_task_full(auto make_task)
{
    simple_executor ex("TestExecutor");
    ex.reserve(32);

    bool done = false;

    std::thread executor_thread([&ex, &done]() {
        while (!done)
            ex.run();
    });

    // Let infrastructure settle
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    reset_allocations();
    auto t = make_task();
    t.set_executor(ex);
    t.set_done_flag(done);
    ex.dispatch([&t]() { t.start(); });

    // Wait for completion
    while (!done)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    stop_tracking();
    size_t allocs = get_allocations();

    ex.stop();
    executor_thread.join();

    return allocs;
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

int main()
{
    std::cout << "demo_my_task: Custom task with affine mixins\n";
    std::cout << "=============================================\n";
#if defined(__clang__)
    std::cout << "Compiler: Clang " << __clang_major__ << "." << __clang_minor__ << "\n\n";
#elif defined(_MSC_VER)
    std::cout << "Compiler: MSVC " << _MSC_VER << "\n\n";
#else
    std::cout << "Compiler: Unknown\n\n";
#endif

    thread_pool pool(2);
    g_pool = &pool;

    size_t empty_allocs = 0;
    size_t affine_allocs = 0;
    size_t legacy_allocs = 0;
    size_t mixed_allocs = 0;

    // Test 0: Empty coroutine (just frame allocation)
    empty_allocs = run_task_full([]{ return empty_task(); });

    // Test 1: Affine awaitables only
    affine_allocs = run_task_full([]{ return test_affine_only(); });

    // Test 2: Legacy awaitables only
    legacy_allocs = run_task_full([]{ return test_legacy_only(); });

    // Test 3: Mixed
    mixed_allocs = run_task_full([]{ return test_mixed(); });

    // Calculate expected differences
    size_t legacy_overhead = legacy_allocs - affine_allocs;
    size_t mixed_overhead = mixed_allocs - affine_allocs;

    // empty_allocs is the pure frame cost (0 with HALO, 1 without)
    bool halo_working = (empty_allocs == 0);
    bool affine_ok = (affine_allocs == empty_allocs);  // No extra allocs for affine awaits
    bool legacy_ok = (legacy_overhead == 3);
    bool mixed_ok = (mixed_overhead == 1);

    g_results.push_back({"HALO (0 = elided, 1 = allocated)", empty_allocs, halo_working});
    g_results.push_back({"3 affine awaits (no overhead)", affine_allocs, affine_ok});
    g_results.push_back({"3 legacy awaits (+3 trampolines)", legacy_allocs, legacy_ok});
    g_results.push_back({"2 affine + 1 legacy (+1 trampoline)", mixed_allocs, mixed_ok});

    // Print results
    std::cout << "Test Results:\n";
    std::cout << "-------------\n";
    std::cout << "  empty coroutine:       " << empty_allocs << " allocs ("
              << (halo_working ? "HALO!" : "no HALO") << ")\n";
    std::cout << "  3 affine awaits:       " << affine_allocs << " allocs\n";
    std::cout << "  3 legacy awaits:       " << legacy_allocs << " allocs (+" 
              << legacy_overhead << " trampolines)\n";
    std::cout << "  2 affine + 1 legacy:   " << mixed_allocs << " allocs (+" 
              << mixed_overhead << " trampolines)\n";

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
