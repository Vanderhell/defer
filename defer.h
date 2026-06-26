/*
 * defer.h - Automatic resource cleanup for C
 *
 * Single header, zero allocation, cleanup-attribute based scope guards.
 *
 * Public API:
 *   DEFER(fn, ctx)
 *   DEFER_NAMED(name, fn, ctx)
 *   DEFER_DISMISS(name)
 *
 * Optional helpers are opt-in:
 *   DEFER_ENABLE_FREE_HELPER
 *   DEFER_ENABLE_STDIO_HELPER
 *   DEFER_ENABLE_UNISTD_HELPER
 *   DEFER_WITH_PTHREAD
 *
 * Unsupported compilers do not get fallback macros unless the caller
 * explicitly defines DEFER_ALLOW_NOOP_FALLBACK.
 */

#ifndef DEFER_H
#define DEFER_H

/* version */
#define DEFER_VERSION_MAJOR 1
#define DEFER_VERSION_MINOR 0
#define DEFER_VERSION_PATCH 0
#define DEFER_VERSION "1.0.0"

/* feature detection */
#if defined(__has_attribute)
#  if __has_attribute(cleanup)
#    define DEFER_SUPPORTED 1
#  else
#    define DEFER_SUPPORTED 0
#  endif
#elif defined(__GNUC__)
#  if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#    define DEFER_SUPPORTED 1
#  else
#    define DEFER_SUPPORTED 0
#  endif
#else
#  define DEFER_SUPPORTED 0
#endif

#if defined(__clang__)
/* Clang treats __COUNTER__ as a C2y extension; this header uses it
 * intentionally for unique guard names on supported compilers. */
#  pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#if defined(__COUNTER__)
#  define DEFER_DETAIL_UNIQUE_ID __COUNTER__
#else
#  define DEFER_DETAIL_UNIQUE_ID __LINE__
#endif

#define DEFER_DETAIL_CONCAT_IMPL(a, b) a##b
#define DEFER_DETAIL_CONCAT(a, b) DEFER_DETAIL_CONCAT_IMPL(a, b)

#if DEFER_SUPPORTED

typedef void (*defer_detail_cleanup_fn)(void *);

typedef struct defer_detail_guard_s {
    defer_detail_cleanup_fn fn;
    void *ctx;
    unsigned active;
} defer_detail_guard_t;

static inline void defer_detail_guard_cleanup(defer_detail_guard_t *guard)
{
    if (guard != 0 && guard->active && guard->fn != 0) {
        guard->active = 0u;
        guard->fn(guard->ctx);
    }
}

#define DEFER_DETAIL_DECLARE_GUARD(name, fn, ctx)                              \
    defer_detail_guard_t                                                      \
        __attribute__((__cleanup__(defer_detail_guard_cleanup))) name = {      \
            (fn), (ctx), 1u                                                    \
        }

#define DEFER(fn, ctx)                                                         \
    DEFER_DETAIL_DECLARE_GUARD(                                                \
        DEFER_DETAIL_CONCAT(defer_detail_guard_, DEFER_DETAIL_UNIQUE_ID),      \
        (fn),                                                                  \
        (ctx))

#define DEFER_NAMED(name, fn, ctx)                                             \
    DEFER_DETAIL_DECLARE_GUARD(name, (fn), (ctx))

#define DEFER_DISMISS(name) ((name).active = 0u)

#if defined(DEFER_ENABLE_FREE_HELPER)
#  include <stdlib.h>

typedef struct defer_detail_free_state_s {
    const void *ptr;
} defer_detail_free_state_t;

static inline void defer_detail_free_cleanup(void *ctx)
{
    const defer_detail_free_state_t *state = (const defer_detail_free_state_t *)ctx;
    if (state != 0 && state->ptr != 0)
        free((void *)state->ptr);
}

#  define DEFER_DETAIL_DECLARE_FREE(ptr_expr, id)                              \
    defer_detail_free_state_t DEFER_DETAIL_CONCAT(defer_detail_free_state_, id) = { \
        (const void *)(ptr_expr)                                               \
    };                                                                         \
    DEFER_DETAIL_DECLARE_GUARD(                                                \
        DEFER_DETAIL_CONCAT(defer_detail_free_guard_, id),                     \
        defer_detail_free_cleanup,                                             \
        &DEFER_DETAIL_CONCAT(defer_detail_free_state_, id))

#  define DEFER_FREE(ptr_expr) DEFER_DETAIL_DECLARE_FREE((ptr_expr), DEFER_DETAIL_UNIQUE_ID)
#endif

#if defined(DEFER_ENABLE_STDIO_HELPER)
#  include <stdio.h>

typedef struct defer_detail_fclose_state_s {
    FILE *fp;
} defer_detail_fclose_state_t;

static inline void defer_detail_fclose_cleanup(void *ctx)
{
    const defer_detail_fclose_state_t *state = (const defer_detail_fclose_state_t *)ctx;
    if (state != 0 && state->fp != 0)
        fclose(state->fp);
}

#  define DEFER_DETAIL_DECLARE_FCLOSE(fp_expr, id)                             \
    defer_detail_fclose_state_t DEFER_DETAIL_CONCAT(defer_detail_fclose_state_, id) = { \
        (fp_expr)                                                              \
    };                                                                         \
    DEFER_DETAIL_DECLARE_GUARD(                                                \
        DEFER_DETAIL_CONCAT(defer_detail_fclose_guard_, id),                   \
        defer_detail_fclose_cleanup,                                           \
        &DEFER_DETAIL_CONCAT(defer_detail_fclose_state_, id))

#  define DEFER_FCLOSE(fp_expr) DEFER_DETAIL_DECLARE_FCLOSE((fp_expr), DEFER_DETAIL_UNIQUE_ID)
#endif

#if defined(DEFER_ENABLE_UNISTD_HELPER)
#  include <unistd.h>

typedef struct defer_detail_close_state_s {
    int fd;
} defer_detail_close_state_t;

static inline void defer_detail_close_cleanup(void *ctx)
{
    const defer_detail_close_state_t *state = (const defer_detail_close_state_t *)ctx;
    if (state != 0 && state->fd >= 0)
        (void)close(state->fd);
}

#  define DEFER_DETAIL_DECLARE_CLOSE(fd_expr, id)                              \
    defer_detail_close_state_t DEFER_DETAIL_CONCAT(defer_detail_close_state_, id) = { \
        (fd_expr)                                                              \
    };                                                                         \
    DEFER_DETAIL_DECLARE_GUARD(                                                \
        DEFER_DETAIL_CONCAT(defer_detail_close_guard_, id),                    \
        defer_detail_close_cleanup,                                            \
        &DEFER_DETAIL_CONCAT(defer_detail_close_state_, id))

#  define DEFER_CLOSE(fd_expr) DEFER_DETAIL_DECLARE_CLOSE((fd_expr), DEFER_DETAIL_UNIQUE_ID)
#endif

#if defined(DEFER_WITH_PTHREAD)
#  include <pthread.h>

typedef struct defer_detail_unlock_state_s {
    pthread_mutex_t *mutex;
} defer_detail_unlock_state_t;

static inline void defer_detail_unlock_cleanup(void *ctx)
{
    const defer_detail_unlock_state_t *state = (const defer_detail_unlock_state_t *)ctx;
    if (state != 0 && state->mutex != 0)
        (void)pthread_mutex_unlock(state->mutex);
}

#  define DEFER_DETAIL_DECLARE_UNLOCK(mutex_expr, id)                          \
    defer_detail_unlock_state_t DEFER_DETAIL_CONCAT(defer_detail_unlock_state_, id) = { \
        (mutex_expr)                                                           \
    };                                                                         \
    DEFER_DETAIL_DECLARE_GUARD(                                                \
        DEFER_DETAIL_CONCAT(defer_detail_unlock_guard_, id),                   \
        defer_detail_unlock_cleanup,                                           \
        &DEFER_DETAIL_CONCAT(defer_detail_unlock_state_, id))

#  define DEFER_UNLOCK(mutex_expr) DEFER_DETAIL_DECLARE_UNLOCK((mutex_expr), DEFER_DETAIL_UNIQUE_ID)
#endif

#else /* !DEFER_SUPPORTED */

#if defined(DEFER_ALLOW_NOOP_FALLBACK)

#  define DEFER(fn, ctx) ((void)0)
#  define DEFER_NAMED(name, fn, ctx) ((void)0)
#  define DEFER_DISMISS(name) ((void)0)

#  if defined(DEFER_ENABLE_FREE_HELPER)
#    define DEFER_FREE(ptr_expr) ((void)0)
#  endif

#  if defined(DEFER_ENABLE_STDIO_HELPER)
#    define DEFER_FCLOSE(fp_expr) ((void)0)
#  endif

#  if defined(DEFER_ENABLE_UNISTD_HELPER)
#    define DEFER_CLOSE(fd_expr) ((void)0)
#  endif

#  if defined(DEFER_WITH_PTHREAD)
#    define DEFER_UNLOCK(mutex_expr) ((void)0)
#  endif

#endif /* DEFER_ALLOW_NOOP_FALLBACK */

#endif /* DEFER_SUPPORTED */

#endif /* DEFER_H */
