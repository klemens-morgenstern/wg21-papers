//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/wg21-papers/coro-first-io
//

#include "bench_cb.hpp"
#include "bench_co.hpp"

#include <chrono>
#include <iostream>
#include <cstdlib>
#include <new>

static std::size_t g_alloc_count = 0;

void* operator new(std::size_t size)
{
    ++g_alloc_count;
    void* p = std::malloc(size);
    if(!p) throw std::bad_alloc();
    return p;
}

void operator delete(void* p) noexcept
{
    std::free(p);
}

void operator delete(void* p, std::size_t) noexcept
{
    std::free(p);
}

struct bench_test
{
    static constexpr int N = 100000;

    template<class Socket, class AsyncOp>
    static void bench(char const* name, Socket& sock, AsyncOp op)
    {
        using clock = std::chrono::high_resolution_clock;
        auto& ioc = *sock.get_executor().ctx_;
        int count = 0;

        g_alloc_count = 0;
        auto t0 = clock::now();
        for (int i = 0; i < N; ++i)
        {
            op(sock, [&count]{ ++count; });
            ioc.run();
        }
        auto t1 = clock::now();
        auto allocs = g_alloc_count;

        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        std::cout << name << (ns / N) << " ns/op, " << (allocs / N) << " allocs/op\n";
    }

    template<class MakeTask>
    static void bench_co(char const* name, io_context& ioc, MakeTask make_task)
    {
        using clock = std::chrono::high_resolution_clock;
        int count = 0;

        g_alloc_count = 0;
        auto t0 = clock::now();
        for (int i = 0; i < N; ++i)
        {
            co::async_run(ioc.get_executor(), make_task(count));
            ioc.run();
        }
        auto t1 = clock::now();
        auto allocs = g_alloc_count;

        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        std::cout << name << (ns / N) << " ns/op, " << (allocs / N) << " allocs/op\n";
    }

    void
    run()
    {
        std::cout << "\n";

        io_context ioc;
        cb::socket cb_sock(ioc.get_executor());
        co::socket co_sock;

        // 1 call
        bench("read_some        callback: ", cb_sock,
            [](auto& sock, auto h){ sock.async_read_some(std::move(h)); });
        bench_co("read_some        coro:     ", ioc,
            [&](int& count) -> co::task { co_await co_sock.async_read_some(); ++count; });

        std::cout << "\n";

        // 10 calls
        bench("async_read       callback: ", cb_sock,
            [](auto& sock, auto h){ cb::async_read(sock, std::move(h)); });
        bench_co("async_read       coro:     ", ioc,
            [&](int& count) -> co::task { co_await co::async_read(co_sock); ++count; });

        std::cout << "\n";

        // 100 calls
        bench("async_request    callback: ", cb_sock,
            [](auto& sock, auto h){ cb::async_request(sock, std::move(h)); });
        bench_co("async_request    coro:     ", ioc,
            [&](int& count) -> co::task { co_await co::async_request(co_sock); ++count; });

        std::cout << "\n";

        // 1000 calls
        bench("async_session    callback: ", cb_sock,
            [](auto& sock, auto h){ cb::async_session(sock, std::move(h)); });
        bench_co("async_session    coro:     ", ioc,
            [&](int& count) -> co::task { co_await co::async_session(co_sock); ++count; });
    }
};

int main()
{
    bench_test t;
    t.run();
    return 0;
}
