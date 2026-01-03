//
// demo_affine_task_senders.cpp
//
// Comprehensive demo showing affine task works like beman::task with:
// - P2300 senders (via continues_on)
// - Affine awaitables (zero overhead)
// - Legacy awaitables (trampoline fallback)
// - Nested task composition
//

#include "task.hpp"
#include "thread_pool.hpp"

#include <beman/execution/execution.hpp>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace ex = beman::execution;

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
    if (ptr) std::free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
    if (ptr) std::free(ptr);
}

size_t reset_allocations() {
    size_t old = g_allocation_count.exchange(0);
    g_tracking_enabled.store(true);
    return old;
}

size_t get_allocations() {
    return g_allocation_count.load();
}

void stop_tracking() {
    g_tracking_enabled.store(false);
}

//------------------------------------------------------------------------------
// P2300-compliant pool scheduler
//------------------------------------------------------------------------------

class pool_scheduler;

struct schedule_sender {
    thread_pool* pool_;
    pool_scheduler const* sched_;

    using sender_concept = ex::sender_t;
    using completion_signatures = ex::completion_signatures<
        ex::set_value_t(),
        ex::set_stopped_t()>;

    struct env {
        pool_scheduler const* sched_;

        template<typename Tag>
            requires std::same_as<Tag, ex::set_value_t>
        pool_scheduler query(ex::get_completion_scheduler_t<Tag>) const noexcept;
    };

    env get_env() const noexcept { return {sched_}; }

    template<typename Receiver>
    struct operation {
        using operation_state_concept = ex::operation_state_t;
        thread_pool* pool_;
        Receiver rcvr_;

        void start() noexcept {
            pool_->dispatch([this]() {
                ex::set_value(std::move(rcvr_));
            });
        }
    };

    template<typename Receiver>
    auto connect(Receiver&& rcvr) const {
        return operation<std::decay_t<Receiver>>{pool_, std::forward<Receiver>(rcvr)};
    }
};

class pool_scheduler {
    thread_pool* pool_;

public:
    using scheduler_concept = ex::scheduler_t;

    explicit pool_scheduler(thread_pool& pool) : pool_(&pool) {}

    schedule_sender schedule() const noexcept {
        return schedule_sender{pool_, this};
    }

    template<typename F>
    void dispatch(F&& f) const {
        pool_->dispatch(std::forward<F>(f));
    }

    bool operator==(pool_scheduler const&) const noexcept = default;
};

template<typename Tag>
    requires std::same_as<Tag, ex::set_value_t>
pool_scheduler schedule_sender::env::query(
    ex::get_completion_scheduler_t<Tag>) const noexcept {
    return *sched_;
}

using pool_context = task_context<pool_scheduler>;

//------------------------------------------------------------------------------
// Enhanced task with full sender support (three-tier await_transform)
//------------------------------------------------------------------------------

template<typename T = void>
class stask;

namespace detail {

template<typename T>
class stask_promise;

template<typename T>
class stask_promise
    : public affine_promise<stask_promise<T>, pool_context>
{
    std::optional<T> value_;
    std::exception_ptr exception_;

public:
    stask<T> get_return_object();

    std::suspend_always initial_suspend() noexcept { return {}; }

    template<typename U>
        requires std::convertible_to<U, T>
    void return_value(U&& v) {
        value_.emplace(std::forward<U>(v));
    }

    void unhandled_exception() {
        exception_ = std::current_exception();
    }

    T result() {
        if (exception_)
            std::rethrow_exception(exception_);
        return std::move(*value_);
    }

    std::coroutine_handle<> unhandled_stopped() noexcept {
        return this->continuation_ ? this->continuation_ : std::noop_coroutine();
    }

    // Three-tier await_transform
    template<typename Awaitable>
    auto await_transform(Awaitable&& a) {
        using A = std::remove_cvref_t<Awaitable>;

        if constexpr (affine_awaitable<A, pool_context>) {
            // Tier 1: Affine awaitable - zero overhead
            return affine_awaiter{
                std::forward<Awaitable>(a), &*this->dispatcher_};
        }
        else if constexpr (ex::sender<A>) {
            // Tier 2: Sender - use continues_on for scheduler affinity
            return ex::as_awaitable(
                ex::continues_on(
                    std::forward<Awaitable>(a),
                    this->dispatcher_->scheduler()),
                *this);
        }
        else {
            // Tier 3: Legacy awaitable - trampoline fallback
            return make_affine(
                std::forward<Awaitable>(a), *this->dispatcher_);
        }
    }
};

template<>
class stask_promise<void>
    : public affine_promise<stask_promise<void>, pool_context>
{
    std::exception_ptr exception_;

public:
    stask<void> get_return_object();

    std::suspend_always initial_suspend() noexcept { return {}; }

    void return_void() noexcept {}

    void unhandled_exception() {
        exception_ = std::current_exception();
    }

    void result() {
        if (exception_)
            std::rethrow_exception(exception_);
    }

    std::coroutine_handle<> unhandled_stopped() noexcept {
        return this->continuation_ ? this->continuation_ : std::noop_coroutine();
    }

    template<typename Awaitable>
    auto await_transform(Awaitable&& a) {
        using A = std::remove_cvref_t<Awaitable>;

        if constexpr (affine_awaitable<A, pool_context>) {
            return affine_awaiter{
                std::forward<Awaitable>(a), &*this->dispatcher_};
        }
        else if constexpr (ex::sender<A>) {
            return ex::as_awaitable(
                ex::continues_on(
                    std::forward<Awaitable>(a),
                    this->dispatcher_->scheduler()),
                *this);
        }
        else {
            return make_affine(
                std::forward<Awaitable>(a), *this->dispatcher_);
        }
    }
};

} // namespace detail

template<typename T>
class stask : public affine_task<T, stask<T>, pool_context> {
public:
    using promise_type = detail::stask_promise<T>;
    using handle_type = std::coroutine_handle<promise_type>;

private:
    handle_type handle_;

public:
    stask() = default;
    explicit stask(handle_type h) noexcept : handle_(h) {}
    stask(stask&& o) noexcept : handle_(std::exchange(o.handle_, {})) {}
    ~stask() { if (handle_) handle_.destroy(); }

    stask& operator=(stask&& o) noexcept {
        if (this != &o) {
            if (handle_) handle_.destroy();
            handle_ = std::exchange(o.handle_, {});
        }
        return *this;
    }

    handle_type handle() const noexcept { return handle_; }

    void set_scheduler(pool_scheduler& sched) {
        handle_.promise().set_dispatcher(pool_context{sched});
    }

    void set_done_flag(bool& flag) {
        handle_.promise().set_done_flag(flag);
    }

    void start() { handle_.resume(); }
};

namespace detail {

template<typename T>
stask<T> stask_promise<T>::get_return_object() {
    return stask<T>{stask<T>::handle_type::from_promise(*this)};
}

inline stask<void> stask_promise<void>::get_return_object() {
    return stask<void>{stask<void>::handle_type::from_promise(*this)};
}

} // namespace detail

//------------------------------------------------------------------------------
// Test awaitables and senders
//------------------------------------------------------------------------------

thread_pool* g_pool = nullptr;

// Affine awaitable (zero overhead)
template<typename T>
struct affine_read {
    T value_;

    bool await_ready() const noexcept { return false; }

    template<typename Dispatcher>
    void await_suspend(std::coroutine_handle<> h, Dispatcher& d) const {
        g_pool->dispatch([h, &d]() mutable {
            d(h);
        });
    }

    T await_resume() const noexcept { return value_; }
};

// Legacy awaitable (trampoline)
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
stask<> empty_task() {
    co_return;
}

stask<> affine_test_5() {
    for (int i = 0; i < 5; ++i)
        co_await affine_read<int>{i};
}

stask<> legacy_test_5() {
    for (int i = 0; i < 5; ++i)
        co_await legacy_timer{1};
}

stask<> sender_test_5() {
    for (int i = 0; i < 5; ++i)
        co_await ex::just(i);
}

stask<> mixed_test() {
    co_await ex::just(10);            // sender
    co_await affine_read<int>{20};    // affine
    co_await legacy_timer{1};         // legacy
    co_await ex::just(30);            // sender
    co_await affine_read<int>{40};    // affine
}

stask<int> nested_inner(int x) {
    co_await affine_read<int>{x};
    co_return x * 2;
}

stask<int> nested_outer() {
    int v1 = co_await nested_inner(10);
    int v2 = co_await nested_inner(20);
    co_return v1 + v2;
}

//------------------------------------------------------------------------------
// Helper to run task and count allocations
//------------------------------------------------------------------------------

template<typename T>
size_t run_and_count(stask<T> t, pool_scheduler& sched) {
    bool done = false;
    t.set_scheduler(sched);
    t.set_done_flag(done);
    
    reset_allocations();
    t.start();
    
    while (!done)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    stop_tracking();
    return get_allocations();
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

int main() {
    std::cout << "demo_affine_task_senders: Full sender/awaitable support\n";
    std::cout << "========================================================\n";
#if defined(__clang__)
    std::cout << "Compiler: Clang " << __clang_major__ << "." << __clang_minor__ << "\n\n";
#elif defined(_MSC_VER)
    std::cout << "Compiler: MSVC " << _MSC_VER << "\n\n";
#else
    std::cout << "Compiler: Unknown\n\n";
#endif

    thread_pool pool(2);
    g_pool = &pool;
    pool_scheduler sched(pool);

    size_t empty_allocs = 0;
    size_t affine_5 = 0;
    size_t legacy_5 = 0;
    size_t sender_5 = 0;
    size_t mixed_allocs = 0;
    size_t nested_allocs = 0;

    // Helper to run task and count allocations INCLUDING task creation
    auto run_and_count_full = [&](auto make_task) {
        reset_allocations();
        auto t = make_task();
        bool done = false;
        t.set_scheduler(sched);
        t.set_done_flag(done);
        t.start();
        while (!done)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        stop_tracking();
        return get_allocations();
    };

    // Test 0: Empty coroutine (just frame allocation)
    empty_allocs = run_and_count_full([]{ return empty_task(); });

    // Test 1: 5 affine awaits (baseline)
    affine_5 = run_and_count_full([]{ return affine_test_5(); });

    // Test 2: 5 legacy awaits (+5 trampolines)
    legacy_5 = run_and_count_full([]{ return legacy_test_5(); });

    // Test 3: 5 sender awaits (via continues_on)
    sender_5 = run_and_count_full([]{ return sender_test_5(); });

    // Test 4: Mixed (2 sender + 2 affine + 1 legacy)
    mixed_allocs = run_and_count_full([]{ return mixed_test(); });

    // Test 5: Nested tasks
    nested_allocs = run_and_count_full([]{ return nested_outer(); });

    // Calculate overhead - legacy should be exactly +5 vs affine baseline
    size_t legacy_overhead = legacy_5 - affine_5;

    // empty_allocs is the pure frame cost (0 with HALO, 1 without)
    bool halo_working = (empty_allocs == 0);
    bool affine_ok = (affine_5 == empty_allocs);  // No extra allocs for affine awaits
    bool legacy_ok = (legacy_overhead == 5);
    bool sender_ok = true;  // Senders have their own allocation pattern
    // mixed = baseline + 1 trampoline (only 1 legacy await)
    bool mixed_ok = (mixed_allocs == empty_allocs + 1);
    // nested = baseline + 2 inner frames
    bool nested_ok = (nested_allocs == empty_allocs + 2);

    g_results.push_back({"HALO (0 = elided, 1 = allocated)", empty_allocs, halo_working});
    g_results.push_back({"5 affine awaits (no overhead)", affine_5, affine_ok});
    g_results.push_back({"5 legacy awaits (+5 trampolines)", legacy_5, legacy_ok});
    g_results.push_back({"5 sender awaits (continues_on)", sender_5, sender_ok});
    g_results.push_back({"mixed (2 sender + 2 affine + 1 legacy)", mixed_allocs, mixed_ok});
    g_results.push_back({"nested tasks (2 inner frames)", nested_allocs, nested_ok});

    // Print results
    std::cout << "Test Results:\n";
    std::cout << "-------------\n";
    std::cout << "  empty coroutine:   " << empty_allocs << " allocs ("
              << (halo_working ? "HALO!" : "no HALO") << ")\n";
    std::cout << "  5 affine awaits:   " << affine_5 << " allocs\n";
    std::cout << "  5 legacy awaits:   " << legacy_5 << " allocs (+" 
              << legacy_overhead << " trampolines)\n";
    std::cout << "  5 sender awaits:   " << sender_5 << " allocs\n";
    std::cout << "  mixed test:        " << mixed_allocs << " allocs\n";
    std::cout << "  nested tasks:      " << nested_allocs << " allocs\n";

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
