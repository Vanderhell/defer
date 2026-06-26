# defer.h

`defer.h` is a single-header scope-guard library for C99 and C11 on compilers
that support the GNU/Clang cleanup attribute.

It is designed for small C projects that want deterministic cleanup without a
runtime registry, heap allocation, or a C++ RAII dependency.

## What it provides

- `DEFER(fn, ctx)` for generic `void (*)(void *)` callbacks
- `DEFER_NAMED(name, fn, ctx)` for explicit scope guards
- `DEFER_DISMISS(name)` for explicit dismissal
- Optional helpers for `free`, `fclose`, `close`, and `pthread_mutex_unlock`
- Explicit unsupported-compiler fallback only when `DEFER_ALLOW_NOOP_FALLBACK`
  is defined before including the header

## Quick start

```c
#define DEFER_ENABLE_FREE_HELPER
#include "defer.h"

void example(void)
{
    char *buf = malloc(128);
    if (buf == NULL)
        return;

    DEFER_FREE(buf);
}
```

For helpers that need hosted or POSIX headers, define the relevant opt-in
before including `defer.h`:

- `DEFER_ENABLE_FREE_HELPER`
- `DEFER_ENABLE_STDIO_HELPER`
- `DEFER_ENABLE_UNISTD_HELPER`
- `DEFER_WITH_PTHREAD`

The core `DEFER` and `DEFER_NAMED` macros do not require those headers.

## Named guards

Use `DEFER_NAMED` when cleanup must be dismissed explicitly or when the final
cleanup action depends on manual error handling.

```c
typedef struct file_state_s {
    FILE *fp;
} file_state_t;

static void close_file(void *ctx)
{
    file_state_t *state = ctx;
    if (state->fp != NULL)
        fclose(state->fp);
}

file_state_t state = { fopen("data.txt", "rb") };
if (state.fp == NULL)
    return;

DEFER_NAMED(file_guard, close_file, &state);

if (fclose(state.fp) != 0)
    return;

state.fp = NULL;
DEFER_DISMISS(file_guard);
```

## Behavior

- Cleanup runs on normal scope exit, `return`, `goto` out of scope, `break`, and
  `continue` when the guarded declaration is left behind.
- Cleanup does not run on `longjmp`, `abort`, `_Exit`, process termination,
  watchdog reset, MCU reset, hard fault, or power loss.
- Jumping into a guarded scope past a defer declaration is forbidden.
- Cleanup callbacks cannot propagate errors. Use explicit cleanup plus
  dismissal when error reporting matters.

## Supported compilers

| Compiler | Status |
|---|---|
| GCC with cleanup attribute support | Supported |
| Clang with cleanup attribute support | Supported |
| Pure MSVC | Not supported unless an explicit no-op fallback is enabled |

`DEFER_SUPPORTED` is `1` only when the cleanup attribute is available.

## Build

The repository includes CMake and a Makefile.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Examples:

```sh
cmake --build build --target example_memory
cmake --build build --target example_files
cmake --build build --target example_mutex
```

## Tests

The test suite covers:

- runtime cleanup behavior
- reassignment after registration
- named guard dismissal
- multiple guards on one line
- multi-translation-unit linkage
- compile-fail fixtures for invalid usage
- an external `find_package(defer CONFIG REQUIRED)` consumer

## License

MIT. See [LICENSE](LICENSE).
