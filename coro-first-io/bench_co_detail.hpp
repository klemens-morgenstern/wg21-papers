//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/wg21-papers/coro-first-io
//

#ifndef BENCH_CO_DETAIL_HPP
#define BENCH_CO_DETAIL_HPP

#include <cstddef>
#include <mutex>

namespace co {
namespace detail {

//----------------------------------------------------------
// Frame pool: thread-local with global overflow
// Tracks block sizes to avoid returning undersized blocks

class frame_pool
{
    struct block
    {
        block* next;
        std::size_t size;
    };

    struct global_pool
    {
        std::mutex mtx;
        block* head = nullptr;

        ~global_pool()
        {
            while(head)
            {
                auto p = head;
                head = head->next;
                ::operator delete(p);
            }
        }

        void push(block* b)
        {
            std::lock_guard<std::mutex> lock(mtx);
            b->next = head;
            head = b;
        }

        block* pop(std::size_t n)
        {
            std::lock_guard<std::mutex> lock(mtx);
            block** pp = &head;
            while(*pp)
            {
                if((*pp)->size >= n)
                {
                    block* p = *pp;
                    *pp = p->next;
                    return p;
                }
                pp = &(*pp)->next;
            }
            return nullptr;
        }
    };

    struct local_pool
    {
        block* head = nullptr;

        void push(block* b)
        {
            b->next = head;
            head = b;
        }

        block* pop(std::size_t n)
        {
            block** pp = &head;
            while(*pp)
            {
                if((*pp)->size >= n)
                {
                    block* p = *pp;
                    *pp = p->next;
                    return p;
                }
                pp = &(*pp)->next;
            }
            return nullptr;
        }
    };

    global_pool& global_;

    static local_pool& get_local()
    {
        static thread_local local_pool local;
        return local;
    }

public:
    explicit frame_pool(global_pool& g) : global_(g) {}

    void* allocate(std::size_t n)
    {
        // First try thread-local (with size check)
        if(auto* b = get_local().pop(n))
            return b;

        // Then try global (with size check)
        if(auto* b = global_.pop(n))
            return b;

        // Fall back to heap
        auto* b = static_cast<block*>(::operator new(n));
        b->size = n;
        return b;
    }

    void deallocate(void* p, std::size_t n)
    {
        // Restore the size (was overwritten by alloc_header::ctx)
        auto* b = static_cast<block*>(p);
        b->size = n;
        // Return to thread-local pool
        get_local().push(b);
    }

    // Factory for creating a global pool
    static global_pool& make_global()
    {
        static global_pool pool;
        return pool;
    }
};

} // detail
} // co

#endif
