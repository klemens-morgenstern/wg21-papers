# Cost Analysis: Callback vs Coroutine (Clang 3x Slowdown)

## Benchmark Results

This document analyzes performance differences between callback and coroutine implementations for composed asynchronous I/O operations. The benchmarks measure four abstraction levels, comparing both plain `socket` and `tls_stream` (which doubles I/O operations to simulate TLS overhead).

**Output format:**
- **Level** (1-4): Abstraction depth (1=read_some, 2=read, 3=request, 4=session)
- **Stream type**: `socket` or `tls_stream`
- **Operation**: `read_some`, `read`, `request`, or `session`
- **Style**: `cb` (callback) or `co` (coroutine)
- **Metrics**: Nanoseconds per operation (`ns/op`) and work items per operation (`work/op`)

### Clang Results

```
1 socket     read_some  cb :     4 ns/op, 1 work/op
1 socket     read_some  co :    22 ns/op, 2 work/op
1 tls_stream read_some  cb :    26 ns/op, 2 work/op
1 tls_stream read_some  co :    37 ns/op, 3 work/op

2 socket     read       cb :    47 ns/op, 10 work/op
2 socket     read       co :    60 ns/op, 11 work/op
2 tls_stream read       cb :   159 ns/op, 20 work/op
2 tls_stream read       co :   141 ns/op, 21 work/op

3 socket     request    cb :   677 ns/op, 100 work/op
3 socket     request    co :   425 ns/op, 101 work/op
3 tls_stream request    cb :  2942 ns/op, 200 work/op
3 tls_stream request    co :  1152 ns/op, 201 work/op

4 socket     session    cb : 12504 ns/op, 1000 work/op
4 socket     session    co :  3868 ns/op, 1001 work/op
4 tls_stream session    cb : 16728 ns/op, 2000 work/op
4 tls_stream session    co : 11409 ns/op, 2001 work/op
```

**The Problem:** At level 4 (`session`), callbacks are **3.2x slower** than coroutines (12504 ns vs 3868 ns). Same work count, same I/O operations, but dramatically different performance. This document explains why Clang struggles where MSVC succeeds.

### MSVC Results

```
1 socket     read_some  cb :     4 ns/op, 1 work/op
1 socket     read_some  co :    24 ns/op, 2 work/op
1 tls_stream read_some  cb :    12 ns/op, 2 work/op
1 tls_stream read_some  co :    50 ns/op, 3 work/op

2 socket     read       cb :    61 ns/op, 10 work/op
2 socket     read       co :    89 ns/op, 11 work/op
2 tls_stream read       cb :   172 ns/op, 20 work/op
2 tls_stream read       co :   361 ns/op, 21 work/op

3 socket     request    cb :   674 ns/op, 100 work/op
3 socket     request    co :   701 ns/op, 101 work/op
3 tls_stream request    cb :  1766 ns/op, 200 work/op
3 tls_stream request    co :  3182 ns/op, 201 work/op

4 socket     session    cb :  7172 ns/op, 1000 work/op
4 socket     session    co :  6611 ns/op, 1001 work/op
4 tls_stream session    cb : 19221 ns/op, 2000 work/op
4 tls_stream session    co : 30290 ns/op, 2001 work/op
```

**Key observation:** At level 4 (`session`), callbacks and coroutines perform nearly equally (7172 ns vs 6611 ns, **1.08x ratio**). MSVC's optimizer successfully eliminates the abstraction penalty that Clang cannot.

### GCC Results

```
1 socket     read_some  cb :     5 ns/op, 1 work/op
1 socket     read_some  co :    20 ns/op, 2 work/op
1 tls_stream read_some  cb :    10 ns/op, 2 work/op
1 tls_stream read_some  co :    29 ns/op, 3 work/op

2 socket     read       cb :    50 ns/op, 10 work/op
2 socket     read       co :    66 ns/op, 11 work/op
2 tls_stream read       cb :   101 ns/op, 20 work/op
2 tls_stream read       co :   212 ns/op, 21 work/op

3 socket     request    cb :   473 ns/op, 100 work/op
3 socket     request    co :   560 ns/op, 101 work/op
3 tls_stream request    cb :   982 ns/op, 200 work/op
3 tls_stream request    co :  2071 ns/op, 201 work/op

4 socket     session    cb :  6613 ns/op, 1000 work/op
4 socket     session    co :  5681 ns/op, 1001 work/op
4 tls_stream session    cb : 13904 ns/op, 2000 work/op
4 tls_stream session    co : 20120 ns/op, 2001 work/op
```

**Key observations:**
- **Callbacks faster for simpler operations** (levels 1-3): Callbacks outperform coroutines at shallow abstraction depths.
- **Coroutines faster for complex operations** (level 4 socket): At level 4 socket operations, coroutines are **1.16x faster** than callbacks (5681 ns vs 6613 ns).
- **TLS overhead favors callbacks**: For TLS operations, callbacks consistently outperform coroutines across all levels (13904 ns vs 20120 ns at level 4, **1.45x faster**).

## The Root Cause

**Nested move constructor chains** that Clang cannot optimize away.

Each I/O operation moves a handler chain through 3-4 levels:
- `io_op` move ctor → moves `read_op` → moves `request_op` → moves `session_op` → moves lambda
- **~280 bytes moved per I/O** vs **~24 bytes assigned** for coroutines
- **11.7x more data movement**, but that alone doesn't explain 3x slowdown

The real issue: **Clang generates conservative code** for complex nested template types.

### Callback Model Issues (Clang)

- ✅ **Recycling possible** via `op_cache`
- ❌ **Nested moves** required per I/O operation (~280 bytes moved)
- ❌ **Template type growth** inhibits optimization (`read_op<request_op<session_op<lambda>>>`)
- ❌ **Temporary object churn** on stack (3 levels: `session_op`, `request_op`, `read_op`)
- ❌ **Clang doesn't inline** deeply nested move constructors
- ❌ **Allocation per I/O** - `io_op` allocated even with recycling

**Why callbacks suffer:**
1. **Template type growth** - Handler type grows: `read_op<request_op<session_op<lambda>>>`
2. **Separate code paths** - Each template instantiation creates different code
3. **Temporary object churn** - Stack temporaries created/destroyed at 3 levels
4. **Defensive code generation** - Clang can't prove moves are safe to optimize

### Coroutine Model Advantages

- ✅ **Pre-allocated operation state** - `read_state` reused 1000 times
- ✅ **No nested moves per I/O** - just assignment of handle + executor (~24 bytes)
- ✅ **Type erasure** - `std::coroutine_handle<void>` is fixed size (8 bytes)
- ✅ **No temporary churn** - coroutine state lives in coroutine frame
- ✅ **Better optimization** - compiler sees entire coroutine state machine
- ✅ **Zero allocations per I/O** - operation state pre-allocated in socket

**Why coroutines win:**
1. **Type erasure** - `std::coroutine_handle<void>` is fixed size (8 bytes)
2. **Pre-allocation** - Single `read_state` reused, no construction per I/O
3. **Single frame** - All state in one coroutine frame, compiler sees entire structure
4. **Static optimization** - State machine structure enables better inlining

### Compiler Differences

The callback abstraction penalty is **compiler-dependent**. Benchmark results show:

- **Clang**: Level 4 session → callback **12504 ns** vs coroutine **3868 ns** (**3.2x slower**)
- **MSVC**: Level 4 session → callback **7172 ns** vs coroutine **6611 ns** (**1.08x slower**, essentially equal)
- **GCC**: Level 4 session → callback **6613 ns** vs coroutine **5681 ns** (**1.16x slower**)

**Why MSVC outperforms Clang:**

1. **More aggressive inlining** - MSVC inlines deeply nested move constructors through 3-4 template layers; Clang stops inlining earlier
2. **Better template instantiation optimization** - MSVC optimizes across template instantiations; Clang treats each instantiation more separately
3. **More aggressive temporary elimination** - MSVC eliminates stack temporaries; Clang is more conservative
4. **Less defensive code generation** - MSVC optimizes complex nested types when it can prove safety; Clang generates more defensive code

**GCC's performance characteristics:**

GCC shows an intermediate optimization profile between Clang and MSVC:
- **Better than Clang**: GCC achieves callback performance closer to coroutines (1.16x ratio vs Clang's 3.2x)
- **Worse than MSVC**: GCC doesn't achieve MSVC's near-parity (1.16x vs MSVC's 1.08x)
- **Callbacks excel at shallow levels**: GCC's callback implementation is faster than coroutines for levels 1-3
- **TLS overhead favors callbacks**: GCC callbacks handle TLS operations more efficiently than coroutines across all levels

**The core issue:** Clang's optimizer is more conservative with deeply nested template types and move operations, generating more function calls and data movement. MSVC's optimizer is more aggressive and can see through the abstraction layers, making callbacks nearly as fast as coroutines. GCC falls between these extremes, optimizing shallow callback chains well but showing some penalty at deeper levels.

## TLS Stream Overhead Analysis

The benchmark results include `tls_stream` measurements, which wrap a `socket` and double I/O operations to simulate TLS overhead. Since `tls_stream` doubles I/O operations, we expect ~2x time if overhead is linear.

**Key findings:**

1. **Clang callbacks achieve sublinear scaling** at Level 4 - fixed costs dominate, making doubling I/O operations relatively cheap
2. **MSVC callbacks scale consistently** - aggressive optimization prevents overhead spikes
3. **GCC callbacks show near-perfect linear scaling** - optimal handling of TLS overhead across all levels (1.0-1.05x factors)
4. **Clang coroutines scale better** than MSVC coroutines (1.2-1.7x better overhead factors)
5. **MSVC and GCC coroutines show superlinear overhead** at deeper levels - potential coroutine frame allocation overhead
6. **GCC callbacks outperform coroutines for TLS** - consistent advantage across all abstraction levels

The `tls_stream` analysis reveals that overhead scaling depends on abstraction depth, compiler optimizations, and continuation model - there's no universal winner. GCC demonstrates that callbacks can achieve near-optimal scaling for TLS operations, while coroutines may introduce overhead at deeper abstraction levels.

## Experimental Validation

Added `char buf[256];` to `session_op` to measure move cost impact:

**Results:**
- **Before:** 13273 ns/op
- **After:** 16211 ns/op  
- **Increase:** ~2940 ns (**22% slower**)

**Analysis:**
`session_op` is moved ~1000 times per `async_session` operation. Adding 256 bytes means **256 KB of data movement** per operation (256 bytes × 1000 moves).

**Conclusion:**
- Move operations are a **real and substantial cost**
- Clang is **not optimizing away** nested moves
- Nested move constructor chains are a **significant contributor** to the 3x slowdown

## Cost Comparison

**Callback Path (per I/O operation):**
- Move handler chain into `io_op`: ~92 bytes moved
- Construct `io_op`: ~96 bytes initialized
- Move handler chain out of `io_op`: ~92 bytes moved
- Destroy `io_op`: cleanup
- **Total: ~280 bytes moved/initialized per I/O**

**Coroutine Path (per I/O operation):**
- Assign handle: 8 bytes
- Assign executor: ~16 bytes
- **Total: ~24 bytes assigned per I/O**

**Ratio: ~11.7x more data movement in callback path**

## Conclusion

**Coroutines avoid the callback abstraction penalty** because:

1. **Pre-allocated operation state** - Single `read_state` reused for all I/O operations
2. **No nested moves per I/O** - Operation state is assigned, not moved
3. **Type erasure via coroutine handle** - Fixed-size handle (8 bytes) vs growing handler types
4. **Single coroutine frame** - All state embedded in one frame, compiler can optimize entire state machine
5. **No temporary object creation** - Coroutine state persists in frame, not created/destroyed per operation

The callback model's 3x slowdown **on Clang** comes from nested move constructor chains that Clang doesn't optimize (~280 bytes moved per I/O), template type growth creating separate code paths, temporary object creation/destruction per I/O operation, and complex nested type handling requiring defensive code generation.

**MSVC avoids this penalty** through more aggressive optimization, achieving near-parity with coroutines.

**The coroutine model performs similarly to sender/receiver** - both avoid the nested move cost penalty that plagues the callback model.
