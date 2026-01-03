# Benchmark: Callbacks vs Coroutines

Compares dispatch overhead for composed async operations.

## Design Principles

- **Both must be fully optimized** — recycle and preallocate whenever possible
- **Measure inherent abstraction cost** — not implementation inefficiencies
- Coroutine preallocation is legitimate, not cheating

## Key Constraint

- **Callbacks cannot preallocate** — handler type varies per call site (templated)
- **Coroutines can preallocate** — `std::coroutine_handle<void>` is type-erased (fixed size)
- Callbacks can only recycle via `op_cache`

## Why This Matters

- I/O always allocates operation state
- Continuations almost always perform another async operation
- Recycling is essential for both models

## Building with GCC (Windows/MSYS2)

### Quick Build Command

**Use `bash -c` wrapper** to avoid PowerShell parsing issues and ensure proper MSYS2 environment:

```bash
# Build with full optimizations (including LTO)
bash -c "g++ -std=c++20 -fcoroutines -Wall -Wextra -Wpedantic -O3 -march=native -flto -funroll-loops -DNDEBUG -ffast-math -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -pthread bench.cpp -o build-gcc/bench.exe -flto -s -Wl,--as-needed"

# Run the benchmark
bash -c "./build-gcc/bench.exe"
```

### Key Learnings

1. **GCC is available via MSYS2** (`C:/msys64/ucrt64/bin/g++.exe`)
   - Version: GCC 15.2.0 (Rev8, Built by MSYS2 project)
   - Already in PATH when MSYS2 is installed

2. **PowerShell issues to avoid:**
   - ❌ Don't use `&&` (use `;` instead)
   - ❌ Don't use backslashes in paths with special characters
   - ❌ Complex command lines with many flags cause parsing errors
   - ✅ Use `bash -c "..."` wrapper for complex commands

3. **CMake configuration challenges:**
   - CMake auto-detects MSVC on Windows, not GCC
   - Specifying `-DCMAKE_CXX_COMPILER=g++` doesn't always work
   - Generators (MinGW Makefiles, Unix Makefiles) may not be available
   - **Solution:** Direct `g++` compilation bypasses CMake entirely

4. **Executable format:**
   - GCC builds create Unix-style executables (not native Windows PE)
   - Must run through `bash -c` or MSYS2 environment
   - Cannot run directly in PowerShell (`The specified executable is not a valid application`)

5. **Optimization flags** (from CMakeLists.txt):
   - Compile: `-std=c++20 -fcoroutines -Wall -Wextra -Wpedantic -O3 -march=native -flto -funroll-loops -DNDEBUG -ffast-math -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -pthread`
   - Link: `-flto -s -Wl,--as-needed`

6. **Build directory:**
   - Output goes to `build-gcc/` directory (enforced by CMakeLists.txt check)
   - CMakeLists.txt validates GCC builds are in `build-gcc` directory

### Recommended Workflow

1. **Ensure build directory exists:**
   ```bash
   mkdir -p build-gcc
   ```

2. **Build directly with g++** (faster, fewer issues than CMake):
   ```bash
   bash -c "g++ -std=c++20 -fcoroutines -Wall -Wextra -Wpedantic -O3 -march=native -flto -funroll-loops -DNDEBUG -ffast-math -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -pthread bench.cpp -o build-gcc/bench.exe -flto -s -Wl,--as-needed"
   ```

3. **Run through bash:**
   ```bash
   bash -c "./build-gcc/bench.exe"
   ```

### Why Direct Compilation Works Better

- ✅ No CMake configuration overhead
- ✅ No generator compatibility issues
- ✅ Direct control over compiler flags
- ✅ Faster iteration (single command)
- ✅ Avoids PowerShell command parsing issues
- ✅ Works reliably in MSYS2/bash environment
