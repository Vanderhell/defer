/*
 * defer.h — Automatic resource cleanup for C
 *
 * Single header, zero allocation, GCC/Clang/ARM Cortex-M.
 * No more goto cleanup.
 *
 * MIT License — https://github.com/Vanderhell/defer.h
 *
 * USAGE:
 *
 *   FILE *f = fopen("x.txt", "r");
 *   DEFER_FCLOSE(f);
 *
 *   void *buf = malloc(256);
 *   DEFER_FREE(buf);
 *
 *   // resources released automatically on scope exit (LIFO order)
 *
 * COMPILER SUPPORT:
 *   GCC   3.4+  — full support via __attribute__((cleanup))
 *   Clang 3.0+  — full support via __attribute__((cleanup))
 *   ARM   GCC   — full support (Cortex-M, Cortex-A, ...)
 *   AVR   GCC   — full support
 *   MSVC        — NOT supported, DEFER_SUPPORTED == 0
 */

#ifndef DEFER_H
#define DEFER_H

/* ── version ──────────────────────────────────────────────────────────── */
#define DEFER_VERSION_MAJOR 0
#define DEFER_VERSION_MINOR 1
#define DEFER_VERSION_PATCH 0
#define DEFER_VERSION "0.1.0"

/* ── compiler detection ───────────────────────────────────────────────── */
#if defined(__GNUC__) || defined(__clang__)
#  define DEFER_SUPPORTED 1
#else
#  define DEFER_SUPPORTED 0
#endif

#if DEFER_SUPPORTED

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── internal machinery ───────────────────────────────────────────────── */

/*
 * Internal slot: holds one deferred (function, context) pair.
 * Placed on the stack — zero heap usage.
 */
typedef struct {
    void (*_fn)(void *);
    void *_ctx;
} _defer_slot_t;

/*
 * Called automatically by __attribute__((cleanup)) when the
 * variable goes out of scope. Never call this directly.
 */
static inline void _defer_run(_defer_slot_t *slot)
{
    if (slot->_fn)
        slot->_fn(slot->_ctx);
}

/* Unique variable name per line — avoids clashes in same scope */
#define _DEFER_CAT2(a, b) a##b
#define _DEFER_CAT(a, b)  _DEFER_CAT2(a, b)
#define _DEFER_VAR(pfx)   _DEFER_CAT(pfx, __LINE__)

/* ── public API ───────────────────────────────────────────────────────── */

/*
 * DEFER(fn, ctx)
 *
 * Low-level macro. Schedule fn(ctx) to run when the current scope exits.
 *
 * Contract:
 *   - fn must have exact signature:  void fn(void *)
 *   - ctx is passed as-is, cast to (void *)
 *   - Multiple DEFERs unwind in LIFO order (last declared, first run).
 *   - Supported on: GCC 3.4+, Clang 3.0+, ARM GCC, AVR GCC
 *   - Unsupported on: MSVC, and other compilers without __attribute__((cleanup))
 *
 * Example:
 *   pthread_mutex_lock(&mtx);
 *   DEFER(_defer_mutex_unlock, &mtx);
 */
#define DEFER(fn, ctx)                                                  \
    _defer_slot_t __attribute__((cleanup(_defer_run)))                  \
    _DEFER_VAR(_defer_slot_) = { (void (*)(void *))(fn), (void *)(ctx) }

/* ─── helpers ─────────────────────────────────────────────────────────── */

/* free() wrapper — accepts void** so cleanup attribute can pass &ptr */
static inline void _defer_free(void *pp)
{
    void **p = (void **)pp;
    if (p && *p) {
        free(*p);
        *p = (void *)0;
    }
}

/*
 * DEFER_FREE(ptr)
 *
 * Automatically call free(ptr) on scope exit. Portable helper macro.
 *
 * Contract:
 *   - ptr is a lvalue (local variable, not a temporary).
 *   - After cleanup, ptr is set to NULL (use-after-free protection).
 *   - NULL pointers are safely ignored.
 *   - Available on supported compilers only (see DEFER_SUPPORTED).
 *
 * Example:
 *   void *buf = malloc(256);
 *   if (!buf) return -1;
 *   DEFER_FREE(buf);
 */
#define DEFER_FREE(ptr) \
    DEFER(_defer_free, &(ptr))

/* ── FILE helpers ─────────────────────────────────────────────────────── */
#include <stdio.h>

static inline void _defer_fclose(void *pp)
{
    FILE **fp = (FILE **)pp;
    if (fp && *fp) {
        fclose(*fp);
        *fp = (FILE *)0;
    }
}

/*
 * DEFER_FCLOSE(fp)
 *
 * Automatically call fclose(fp) on scope exit. Portable helper macro.
 *
 * Contract:
 *   - fp is a lvalue (local variable, not a temporary).
 *   - After cleanup, fp is set to NULL.
 *   - NULL pointers are safely ignored.
 *   - Requires <stdio.h> (included automatically by defer.h).
 *   - Available on supported compilers only.
 *
 * Example:
 *   FILE *f = fopen("x.txt", "r");
 *   if (!f) return -1;
 *   DEFER_FCLOSE(f);
 */
#define DEFER_FCLOSE(fp) \
    DEFER(_defer_fclose, &(fp))

/* ── POSIX fd helpers ─────────────────────────────────────────────────── */
#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>

static inline void _defer_close(void *pp)
{
    int *fdp = (int *)pp;
    if (fdp && *fdp >= 0) {
        close(*fdp);
        *fdp = -1;
    }
}

/*
 * DEFER_CLOSE(fd)
 *
 * Automatically call close(fd) on scope exit. POSIX platforms only.
 *
 * Contract:
 *   - fd is a lvalue (local variable, not a temporary).
 *   - After cleanup, fd is set to -1 (closed/invalid state).
 *   - Negative values are safely ignored.
 *   - Requires: __unix__ or __APPLE__ (POSIX platforms).
 *   - Requires <unistd.h> (included automatically when available).
 *   - Available on supported compilers only.
 *
 * Example:
 *   int fd = open("x.txt", O_RDONLY);
 *   if (fd < 0) return -1;
 *   DEFER_CLOSE(fd);
 */
#define DEFER_CLOSE(fd) \
    DEFER(_defer_close, &(fd))

#endif /* __unix__ || __APPLE__ */

/* ── pthread mutex helper ─────────────────────────────────────────────── */
#if defined(DEFER_WITH_PTHREAD)
#include <pthread.h>
#endif

#if defined(DEFER_WITH_PTHREAD)

static inline void _defer_mutex_unlock(void *pp)
{
    pthread_mutex_t *mp = (pthread_mutex_t *)pp;
    if (mp)
        pthread_mutex_unlock(mp);
}

/*
 * DEFER_UNLOCK(mtx_ptr)
 *
 * Automatically call pthread_mutex_unlock(mtx_ptr) on scope exit.
 * Thread-safe resource cleanup helper for mutex-protected critical sections.
 *
 * Contract:
 *   - Available ONLY when DEFER_WITH_PTHREAD is defined before including defer.h.
 *   - mtx_ptr is a pointer to a locked pthread_mutex_t.
 *   - mtx_ptr must remain valid for the entire scope.
 *   - defer.h will include <pthread.h> automatically when DEFER_WITH_PTHREAD is set.
 *   - POSIX/pthread environments only. Requires GCC 3.4+, Clang 3.0+, or ARM GCC.
 *
 * Example:
 *   #define DEFER_WITH_PTHREAD
 *   #include "defer.h"
 *   ...
 *   pthread_mutex_lock(&g_mtx);
 *   DEFER_UNLOCK(&g_mtx);
 */
#define DEFER_UNLOCK(mtx_ptr) \
    DEFER(_defer_mutex_unlock, (mtx_ptr))

#endif /* DEFER_WITH_PTHREAD */

#ifdef __cplusplus
}
#endif

#else /* !DEFER_SUPPORTED */

/*
 * UNSUPPORTED COMPILER FALLBACK
 *
 * The current compiler does not support __attribute__((cleanup)), which is
 * required for defer.h to function.
 *
 * By default, DEFER macros are NOT defined. Using them will produce a
 * compile-time "undefined identifier" error, preventing silent failure of
 * cleanup code.
 *
 * If you understand the risk and explicitly want no-op behavior, define
 * DEFER_ALLOW_NOOP_FALLBACK before including defer.h:
 *
 *   #define DEFER_ALLOW_NOOP_FALLBACK
 *   #include "defer.h"
 *
 * With this flag, DEFER macros become no-ops on unsupported compilers.
 * WARNING: Cleanup code will not run. Use only if you are certain that your
 * application does not rely on deferred cleanup.
 */

#ifdef DEFER_ALLOW_NOOP_FALLBACK

/* Explicit opt-in — macros are no-ops. C99 compatible. */
#define DEFER(fn, ctx)    ((void)0)
#define DEFER_FREE(ptr)   ((void)0)
#define DEFER_FCLOSE(fp)  ((void)0)
#define DEFER_CLOSE(fd)   ((void)0)
#define DEFER_UNLOCK(m)   ((void)0)

#endif /* DEFER_ALLOW_NOOP_FALLBACK */

#endif /* DEFER_SUPPORTED */
#endif /* DEFER_H */
