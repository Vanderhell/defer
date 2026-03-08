# defer.h

> Automatic resource cleanup for C. Single header, zero allocation, no more `goto cleanup`.

[![CI](https://github.com/Vanderhell/defer.h/actions/workflows/ci.yml/badge.svg)](https://github.com/Vanderhell/defer.h/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![C99](https://img.shields.io/badge/C-99-blue.svg)](defer.h)
[![Single header](https://img.shields.io/badge/header-single-orange.svg)](defer.h)
[![ARM](https://img.shields.io/badge/ARM-Cortex--M-red.svg)](defer.h)
[![Zero allocation](https://img.shields.io/badge/allocation-zero-brightgreen.svg)](defer.h)

---

## The problem

Every C programmer knows this pattern:

```c
int process(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    void *buf = malloc(512);
    if (!buf) {
        fclose(f);          /* easy to forget */
        return -1;
    }

    int rc = do_work(f, buf);

    free(buf);              /* must remember order */
    fclose(f);
    return rc;
}
```

Every early return needs a manual cleanup. Miss one `fclose`, leak one `free`,
forget one `unlock` — and you have a bug that only shows up under error conditions.

## The solution

```c
#include "defer.h"

int process(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    DEFER_FCLOSE(f);        /* registered — runs on any exit */

    void *buf = malloc(512);
    if (!buf) return -1;    /* f closed automatically */
    DEFER_FREE(buf);        /* registered — runs on any exit */

    return do_work(f, buf);
    /* buf freed, f closed — automatically, in LIFO order */
}
```

Cleanup is declared next to acquisition. It runs on **any** exit path —
normal return, early return, or any scope exit.

---

## Installation

`defer.h` is a single header file. No build system, no dependencies.

```sh
cp defer.h your_project/
```

```c
#include "defer.h"
```

Done.

---

## API

### `DEFER(fn, ctx)`

Schedule `fn(ctx)` to run when the current scope exits.  
`fn` must have the signature `void fn(void *)`.  
Multiple defers unwind in **LIFO order** (last declared → first run).

```c
static void my_cleanup(void *ctx) {
    my_resource_t *r = ctx;
    resource_free(r);
}

my_resource_t *r = resource_alloc();
DEFER(my_cleanup, r);
```

### `DEFER_FREE(ptr)`

Calls `free(ptr)` on scope exit. NULL-safe. Also NULLs the pointer after
freeing as protection against use-after-free.

```c
void *buf = malloc(256);
if (!buf) return -1;
DEFER_FREE(buf);
```

### `DEFER_FCLOSE(fp)`

Calls `fclose(fp)` on scope exit. NULL-safe.

```c
FILE *f = fopen("data.bin", "rb");
if (!f) return -1;
DEFER_FCLOSE(f);
```

### `DEFER_CLOSE(fd)`

Calls `close(fd)` on scope exit. Safe when `fd < 0`. POSIX only.

```c
int fd = open("data.bin", O_RDONLY);
if (fd < 0) return -1;
DEFER_CLOSE(fd);
```

### `DEFER_UNLOCK(mtx_ptr)`

Calls `pthread_mutex_unlock(mtx_ptr)` on scope exit.  
Requires `#define DEFER_WITH_PTHREAD` before `#include "defer.h"`.

```c
#define DEFER_WITH_PTHREAD
#include "defer.h"

pthread_mutex_lock(&g_mtx);
DEFER_UNLOCK(&g_mtx);
```

---

## LIFO order

Multiple defers in the same scope unwind in reverse order of declaration —
matching standard RAII and C++ destructor semantics:

```c
DEFER(cleanup_a, &a);   /* runs 3rd */
DEFER(cleanup_b, &b);   /* runs 2nd */
DEFER(cleanup_c, &c);   /* runs 1st */
```

Nested scopes clean up the inner scope before the outer:

```c
{                           /* outer scope */
    DEFER(outer_cleanup, x);

    {                       /* inner scope */
        DEFER(inner_cleanup, y);
    }                       /* inner_cleanup runs here */

}                           /* outer_cleanup runs here */
```

---

## Compiler support

| Compiler | Status | Notes |
|---|---|---|
| GCC 3.4+ | ✅ Full | `__attribute__((cleanup))` |
| Clang 3.0+ | ✅ Full | `__attribute__((cleanup))` |
| ARM GCC (Cortex-M/A/R) | ✅ Full | Tested: `arm-none-eabi-gcc` |
| AVR GCC | ✅ Full | |
| MSVC | ❌ Not supported | `DEFER_SUPPORTED == 0`, macros undefined |

### Unsupported compilers

On unsupported compilers (e.g., MSVC), `DEFER_SUPPORTED` is `0`. DEFER macros are
**not defined** by default to prevent silent failure.

If a macro is used without being defined, you get a clear compile-time error:
```c
error: undefined identifier 'DEFER_FREE'
```

If you want no-op fallback (not recommended), explicitly define `DEFER_ALLOW_NOOP_FALLBACK`
before including `defer.h`:

```c
#define DEFER_ALLOW_NOOP_FALLBACK
#include "defer.h"

DEFER_FREE(ptr);  /* becomes a no-op */
```

**Warning:** With this fallback, cleanup code will not run. Use only if you understand the risks.

---

## Examples

See the [`examples/`](examples/) directory:

- [`examples/files.c`](examples/files.c) — `DEFER_FCLOSE` + `DEFER_FREE` with file copy
- [`examples/memory.c`](examples/memory.c) — multiple allocations, early returns
- [`examples/mutex.c`](examples/mutex.c) — `DEFER_UNLOCK` with pthreads

Build and run:

```sh
gcc -std=c11 -Wall -o files  examples/files.c  && ./files
gcc -std=c11 -Wall -o memory examples/memory.c && ./memory
gcc -std=c11 -Wall -lpthread -o mutex examples/mutex.c && ./mutex
```

---

## Running the tests

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -o test tests/test_defer.c && ./test
```

With sanitizers:

```sh
gcc -std=c11 -fsanitize=address,undefined -o test tests/test_defer.c && ./test
```

Expected output:

```
defer.h v0.1.0 — test suite
compiler: gcc 13.3.0
DEFER_SUPPORTED: 1

  defer_free_basic ...                         PASS
  defer_free_zeros_ptr ...                     PASS
  defer_free_null_safe ...                     PASS
  ...
Results: 13/13 passed
```

---

## How it works

`defer.h` uses GCC/Clang's `__attribute__((cleanup(fn)))` — a well-established
compiler extension that calls a function automatically when a variable goes out
of scope. It is available in GCC since 3.4 (2004) and in every Clang release.

Each `DEFER` macro declares a small struct on the **stack** — no heap
allocation, no global state, no runtime overhead beyond a single function call
on exit.

---

## Alternatives

| Alternative | Portable | Zero alloc | Embedded | Clean API |
|---|---|---|---|---|
| `goto cleanup` | ✅ | ✅ | ✅ | ❌ verbose |
| GCC `__attribute__((cleanup))` raw | ✅ | ✅ | ✅ | ❌ boilerplate |
| `defer.h` | ✅ | ✅ | ✅ | ✅ |

---

## License

[MIT](LICENSE) — free for commercial and embedded use.
