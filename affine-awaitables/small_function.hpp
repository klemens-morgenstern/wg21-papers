//
// small_function.hpp
//
// A non-allocating std::function replacement using small buffer optimization.
//

#ifndef SMALL_FUNCTION_HPP
#define SMALL_FUNCTION_HPP

#include <cstddef>
#include <functional>
#include <new>
#include <type_traits>
#include <utility>

/** A non-allocating replacement for std::function.

    This class provides a subset of the functionality of std::function
    but with a fixed size and no heap allocations. It uses small
    buffer optimization (SBO) to store callables up to the specified
    Capacity.

    @tparam Signature The function signature of the callable.
    @tparam Capacity The size of the internal storage in bytes.
*/
template<typename Signature, std::size_t Capacity = 32>
class small_function;

/** A non-allocating replacement for std::function.
*/
template<typename R, typename... Args, std::size_t Capacity>
class small_function<R(Args...), Capacity>
{
    static constexpr std::size_t capacity = Capacity;

    using invoke_fn = R(*)(void*, Args...);
    using destroy_fn = void(*)(void*);
    using move_fn = void(*)(void*, void*);

    alignas(std::max_align_t) unsigned char storage_[capacity];
    invoke_fn invoke_ = nullptr;
    destroy_fn destroy_ = nullptr;
    move_fn move_ = nullptr;

    template<typename F>
    static R
    invoke_impl(void* ptr, Args... args)
    {
        return (*static_cast<F*>(ptr))(std::forward<Args>(args)...);
    }

    template<typename F>
    static void
    destroy_impl(void* ptr)
    {
        static_cast<F*>(ptr)->~F();
    }

    template<typename F>
    static void
    move_impl(void* dst, void* src)
    {
        new(dst) F(std::move(*static_cast<F*>(src)));
        static_cast<F*>(src)->~F();
    }

public:
    small_function() noexcept
        : invoke_(nullptr)
        , destroy_(nullptr)
        , move_(nullptr)
    {
    }

    template<
        typename F,
        typename = std::enable_if_t<
            !std::is_same_v<std::decay_t<F>, small_function> &&
            std::is_invocable_r_v<R, F, Args...>>>
    small_function(F&& f)
    {
        using Fn = std::decay_t<F>;
        static_assert(
            sizeof(Fn) <= capacity,
            "Callable too large for small_function");
        static_assert(
            alignof(Fn) <= alignof(std::max_align_t),
            "Callable alignment too large for small_function");

        new(storage_) Fn(std::forward<F>(f));
        invoke_ = &invoke_impl<Fn>;
        destroy_ = &destroy_impl<Fn>;
        move_ = &move_impl<Fn>;
    }

    small_function(small_function&& other) noexcept
        : invoke_(nullptr)
        , destroy_(nullptr)
        , move_(nullptr)
    {
        if(other.invoke_)
        {
            other.move_(storage_, other.storage_);
            invoke_ = other.invoke_;
            destroy_ = other.destroy_;
            move_ = other.move_;
            other.invoke_ = nullptr;
            other.destroy_ = nullptr;
            other.move_ = nullptr;
        }
    }

    small_function&
    operator=(small_function&& other) noexcept
    {
        if(this != &other)
        {
            if(invoke_)
                destroy_(storage_);

            if(other.invoke_)
            {
                other.move_(storage_, other.storage_);
                invoke_ = other.invoke_;
                destroy_ = other.destroy_;
                move_ = other.move_;
                other.invoke_ = nullptr;
                other.destroy_ = nullptr;
                other.move_ = nullptr;
            }
            else
            {
                invoke_ = nullptr;
                destroy_ = nullptr;
                move_ = nullptr;
            }
        }
        return *this;
    }

    ~small_function()
    {
        if(invoke_)
            destroy_(storage_);
    }

    small_function(small_function const&) = delete;
    small_function& operator=(small_function const&) = delete;

    explicit
    operator bool() const noexcept
    {
        return invoke_ != nullptr;
    }

    R
    operator()(Args... args)
    {
        if (!invoke_)
            throw std::bad_function_call();
        return invoke_(storage_, std::forward<Args>(args)...);
    }
};

#endif

