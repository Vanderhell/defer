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
#  warning "defer.h: __attribute__((cleanup)) not supported on this compiler. DEFER macros are no-ops."
#endif

#if DEFER_SUPPORTED

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
 * Schedule fn(ctx) to run when the current scope exits.
 * fn must have signature: void fn(void *)
 * Multiple DEFERs unwind in LIFO order (last declared, first run).
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
        __builtin_free(*p);
        *p = (void *)0;
    }
}

/*
 * DEFER_FREE(ptr)
 *
 * Calls free(ptr) on scope exit. NULL-safe.
 * Also NULLs the pointer after freeing (use-after-free protection).
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
 * Calls fclose(fp) on scope exit. NULL-safe.
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
 * Calls close(fd) on scope exit. Safe for fd < 0.
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
#if defined(_PTHREAD_H) || defined(DEFER_WITH_PTHREAD)
static inline void _defer_mutex_unlock(void *pp)
{
    pthread_mutex_t *mp = (pthread_mutex_t *)pp;
    if (mp)
        pthread_mutex_unlock(mp);
}

/*
 * DEFER_UNLOCK(mtx_ptr)
 *
 * Calls pthread_mutex_unlock(mtx_ptr) on scope exit.
 * mtx_ptr must be a pointer to pthread_mutex_t.
 *
 * Example:
 *   pthread_mutex_lock(&g_mtx);
 *   DEFER_UNLOCK(&g_mtx);
 */
#define DEFER_UNLOCK(mtx_ptr) \
    DEFER(_defer_mutex_unlock, (mtx_ptr))

#endif /* pthread */

#ifdef __cplusplus
}
#endif

#else /* !DEFER_SUPPORTED */

/* Unsupported compiler — macros are no-ops, code still compiles */
#define DEFER(fn, ctx)    ((void)0)
#define DEFER_FREE(ptr)   ((void)0)
#define DEFER_FCLOSE(fp)  ((void)0)
#define DEFER_CLOSE(fd)   ((void)0)
#define DEFER_UNLOCK(m)   ((void)0)

#endif /* DEFER_SUPPORTED */
#endif /* DEFER_H */
