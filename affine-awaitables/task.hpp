//
// task.hpp
//
// A complete coroutine task type built on affine awaitable primitives.
// Demonstrates that P3552's task can be implemented using affine_promise
// and affine_task mixins, supporting scheduler affinity for all awaitables.
//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TASK_HPP
#define TASK_HPP

#include "affine.hpp"
#include "affine_helpers.hpp"
#include "make_affine.hpp"

#include <cassert>
#include <exception>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

/** Unified context serving as both dispatcher and scheduler.

    This class wraps a scheduler and provides a unified interface
    that works with both P2300 senders and traditional awaitables.
    It satisfies the dispatcher concept (callable with coroutine handles)
    while also providing access to the underlying scheduler for sender
    operations like continues_on.

    @par Dispatcher Interface
    The class satisfies the dispatcher concept:
    @code
    task_context<MyScheduler> ctx{scheduler};
    ctx(h);  // Dispatches coroutine handle via scheduler
    @endcode

    @par Scheduler Access
    For P2300 sender operations:
    @code
    auto sender = continues_on(some_sender, ctx.scheduler());
    @endcode

    @par Scheduler Requirements
    The scheduler type must provide a dispatch method:
    @code
    struct Scheduler
    {
        void dispatch(std::coroutine_handle<> h);
    };
    @endcode

    @tparam Scheduler The underlying scheduler type.
*/
template<typename Scheduler>
class task_context {
    Scheduler* sched_ = nullptr;

public:
    task_context() = default;

    explicit task_context(Scheduler& s) noexcept
        : sched_(&s)
    {
    }

    task_context(task_context const&) = default;
    task_context& operator=(task_context const&) = default;

    explicit operator bool() const noexcept {
        return sched_ != nullptr;
    }

    template<typename F>
    void operator()(F&& f) const {
        assert(sched_);
        sched_->dispatch(std::forward<F>(f));
    }

    // Overload for coroutine handles (dispatcher concept requirement)
    std::coroutine_handle<> operator()(std::coroutine_handle<> h) const {
        assert(sched_);
        sched_->dispatch([h]() mutable { h.resume(); });
        return std::noop_coroutine();
    }

    Scheduler& scheduler() const noexcept {
        assert(sched_);
        return *sched_;
    }
};

//------------------------------------------------------------------------------

template<typename T, typename Scheduler>
class task;

namespace detail {

// Base promise with common functionality
template<typename T, typename Scheduler>
class task_promise_base
    : public affine_promise<task_promise_base<T, Scheduler>, task_context<Scheduler>>
{
public:
    using context_type = task_context<Scheduler>;

protected:
    std::exception_ptr exception_;

public:
    std::suspend_always initial_suspend() noexcept { return {}; }

    void unhandled_exception() {
        exception_ = std::current_exception();
    }

    // Three-tier await_transform
    template<typename Awaitable>
    auto await_transform(Awaitable&& a) {
        using A = std::remove_cvref_t<Awaitable>;

        if constexpr (affine_awaitable<A, context_type>) {
            // Tier 1/2: Affine awaitable (includes senders converted to affine)
            return affine_awaiter{
                std::forward<Awaitable>(a), &*this->dispatcher_};
        } else {
            // Tier 3: Legacy awaitable - use trampoline
            return make_affine(
                std::forward<Awaitable>(a), *this->dispatcher_);
        }
    }
};

// Promise for non-void T
template<typename T, typename Scheduler>
class task_promise : public task_promise_base<T, Scheduler>
{
    std::optional<T> value_;

public:
    task<T, Scheduler> get_return_object();

    template<typename U>
        requires std::convertible_to<U, T>
    void return_value(U&& value) {
        value_.emplace(std::forward<U>(value));
    }

    T result() {
        if (this->exception_)
            std::rethrow_exception(this->exception_);
        return std::move(*value_);
    }
};

// Promise for void
template<typename Scheduler>
class task_promise<void, Scheduler> : public task_promise_base<void, Scheduler>
{
public:
    task<void, Scheduler> get_return_object();

    void return_void() noexcept {}

    void result() {
        if (this->exception_)
            std::rethrow_exception(this->exception_);
    }
};

} // namespace detail

//------------------------------------------------------------------------------

/** A coroutine task type with scheduler affinity.

    This is analogous to P3552's std::execution::task<T>. It is built
    on affine_promise and affine_task mixins to provide:

    - Scheduler affinity: resumes on the designated scheduler after co_await
    - Three-tier dispatch: senders, affine awaitables, legacy awaitables
    - Exception propagation via result()
    - Support for void and non-void return types

    @tparam T The result type of the task.
    @tparam Scheduler The underlying scheduler type.
*/
template<typename T, typename Scheduler>
class task
    : public affine_task<T, task<T, Scheduler>, task_context<Scheduler>>
{
public:
    using promise_type = detail::task_promise<T, Scheduler>;
    using handle_type = std::coroutine_handle<promise_type>;
    using context_type = task_context<Scheduler>;

private:
    handle_type handle_;

public:
    task() = default;

    explicit task(handle_type h) noexcept
        : handle_(h)
    {
    }

    task(task&& other) noexcept
        : handle_(std::exchange(other.handle_, {}))
    {
    }

    task& operator=(task&& other) noexcept {
        if (this != &other) {
            if (handle_)
                handle_.destroy();
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }

    ~task() {
        if (handle_)
            handle_.destroy();
    }

    // Required by affine_task mixin
    handle_type handle() const noexcept { return handle_; }

    // Set the scheduler for affinity
    void set_scheduler(Scheduler& sched) {
        handle_.promise().set_dispatcher(context_type{sched});
    }

    // Set completion flag for sync_wait patterns
    void set_done_flag(bool& flag) {
        handle_.promise().set_done_flag(flag);
    }

    // Start the coroutine
    void start() {
        handle_.resume();
    }

    // Check if valid
    explicit operator bool() const noexcept {
        return handle_ != nullptr;
    }
};

//------------------------------------------------------------------------------

// Deferred definitions
namespace detail {

template<typename T, typename Scheduler>
task<T, Scheduler> task_promise<T, Scheduler>::get_return_object() {
    return task<T, Scheduler>{
        task<T, Scheduler>::handle_type::from_promise(*this)};
}

template<typename Scheduler>
task<void, Scheduler> task_promise<void, Scheduler>::get_return_object() {
    return task<void, Scheduler>{
        task<void, Scheduler>::handle_type::from_promise(*this)};
}

} // namespace detail

//------------------------------------------------------------------------------

// some helpers

/** Block until task completes and return result.

    This is a simple sync_wait that runs a task on a scheduler.
    For production use, a more sophisticated implementation would
    be needed (e.g., with proper run loop integration).

    @param t The task to wait for.
    @param sched The scheduler to use for affinity.
    @param run A callable used to run the scheduler's event loop.
    @return The value produced by the task.
*/
template<typename T, typename Scheduler, typename RunFunc>
T sync_wait(task<T, Scheduler> t, Scheduler& sched, RunFunc&& run) {
    bool done = false;
    t.set_scheduler(sched);
    t.set_done_flag(done);
    t.start();

    // Run the scheduler until done
    while (!done) {
        run();
    }

    return t.handle().promise().result();
}

/** Block until task completes.

    This is a simple sync_wait that runs a task on a scheduler.
    For production use, a more sophisticated implementation would
    be needed (e.g., with proper run loop integration).

    @param t The task to wait for.
    @param sched The scheduler to use for affinity.
    @param run A callable used to run the scheduler's event loop.
*/
template<typename Scheduler, typename RunFunc>
void sync_wait(task<void, Scheduler> t, Scheduler& sched, RunFunc&& run) {
    bool done = false;
    t.set_scheduler(sched);
    t.set_done_flag(done);
    t.start();

    // Run the scheduler until done
    while (!done) {
        run();
    }

    t.handle().promise().result(); // May rethrow
}

#endif // AFFINE_TASK_HPP


