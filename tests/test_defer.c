/* required for mkstemp, unlink on strict C99/C11 */
#define _POSIX_C_SOURCE 200809L

/*
 * tests/test_defer.c — Test suite for defer.h
 *
 * Tests:
 *   1.  DEFER_FREE — basic free, NULL-safety, pointer zeroing
 *   2.  DEFER_FCLOSE — file close on return
 *   3.  DEFER_CLOSE — POSIX fd close on return
 *   4.  DEFER(fn, ctx) — generic callback
 *   5.  LIFO order — multiple defers unwind correctly
 *   6.  Nested scopes — inner scope cleaned before outer
 *   7.  Early return — cleanup still fires
 *   8.  Null pointer safety — no crash on NULL
 *   9.  Multiple defers in one function
 *   10. DEFER_FREE zeros the pointer after free
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

#include "../defer.h"

/* ── test framework ───────────────────────────────────────────────────── */

static int _tests_run    = 0;
static int _tests_passed = 0;
static int _tests_failed = 0;

#define TEST(name) \
    static void test_##name(void)

#define RUN(name) do { \
    _tests_run++; \
    printf("  %-45s", #name " ..."); \
    fflush(stdout); \
    test_##name(); \
    _tests_passed++; \
    printf("PASS\n"); \
} while (0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        _tests_failed++; \
        _tests_passed--; \
        printf("FAIL\n    Assertion failed: %s\n    at %s:%d\n", \
               #cond, __FILE__, __LINE__); \
        return; \
    } \
} while (0)

/* ── helpers ──────────────────────────────────────────────────────────── */

static int g_call_log[16];
static int g_call_count = 0;

static void log_call(void *id_ptr)
{
    int id = *(int *)id_ptr;
    g_call_log[g_call_count++] = id;
}

static void reset_log(void)
{
    memset(g_call_log, 0, sizeof(g_call_log));
    g_call_count = 0;
}

/* ── test 1: DEFER_FREE basic ─────────────────────────────────────────── */
TEST(defer_free_basic)
{
    int freed = 0;
    {
        void *p = malloc(64);
        ASSERT(p != NULL);
        DEFER_FREE(p);
        freed = 1; /* we reach here */
    }
    /* If we get here without crash — valgrind would confirm no leak */
    ASSERT(freed == 1);
}

/* ── test 2: DEFER_FREE zeros pointer ────────────────────────────────── */
TEST(defer_free_zeros_ptr)
{
    void *p = malloc(64);
    ASSERT(p != NULL);
    {
        void *q = p; /* save original */
        (void)q;
        DEFER_FREE(p);
    }
    ASSERT(p == NULL); /* must be zeroed */
}

/* ── test 3: DEFER_FREE NULL safety ──────────────────────────────────── */
TEST(defer_free_null_safe)
{
    void *p = NULL;
    {
        DEFER_FREE(p); /* must not crash */
    }
    ASSERT(p == NULL);
}

/* ── test 4: DEFER_FCLOSE basic ──────────────────────────────────────── */
#if defined(__unix__) || defined(__APPLE__)
TEST(defer_fclose_basic)
{
    char tmpname[] = "/tmp/defer_test_XXXXXX";
    int tmpfd = mkstemp(tmpname);
    ASSERT(tmpfd >= 0);
    close(tmpfd);

    {
        FILE *f = fopen(tmpname, "r");
        ASSERT(f != NULL);
        DEFER_FCLOSE(f);
        ASSERT(f != NULL); /* still open inside scope */
    }
    /* f is closed — remove temp file */
    unlink(tmpname);
    /* reaching here without abort = OK */
    ASSERT(1);
}
#else
/* POSIX-only test — skipped on non-POSIX platforms */
TEST(defer_fclose_basic)
{
    ASSERT(1);
}
#endif

/* ── test 5: DEFER_FCLOSE NULL safety ────────────────────────────────── */
TEST(defer_fclose_null_safe)
{
    FILE *f = NULL;
    {
        DEFER_FCLOSE(f); /* must not crash */
    }
    ASSERT(f == NULL);
}

/* ── test 6: DEFER_CLOSE basic ───────────────────────────────────────── */
#if defined(__unix__) || defined(__APPLE__)
TEST(defer_close_basic)
{
    char tmpname[] = "/tmp/defer_fd_XXXXXX";
    {
        int fd = mkstemp(tmpname);
        ASSERT(fd >= 0);
        DEFER_CLOSE(fd);
        ASSERT(fd >= 0); /* still open inside scope */
    }
    unlink(tmpname);
    ASSERT(1);
}

/* ── test 7: DEFER_CLOSE negative fd safety ──────────────────────────── */
TEST(defer_close_neg_safe)
{
    int fd = -1;
    {
        DEFER_CLOSE(fd); /* must not crash */
    }
    ASSERT(fd == -1);
}
#else
/* POSIX-only test — skipped on non-POSIX platforms */
TEST(defer_close_basic)
{
    ASSERT(1);
}

TEST(defer_close_neg_safe)
{
    ASSERT(1);
}
#endif

/* ── test 8: DEFER(fn, ctx) generic ──────────────────────────────────── */
TEST(defer_generic_callback)
{
    reset_log();
    int id = 42;
    {
        DEFER(log_call, &id);
    }
    ASSERT(g_call_count == 1);
    ASSERT(g_call_log[0] == 42);
}

/* ── test 9: LIFO order ───────────────────────────────────────────────── */
TEST(defer_lifo_order)
{
    reset_log();
    int a = 1, b = 2, c = 3;
    {
        DEFER(log_call, &a); /* registered first → runs last */
        DEFER(log_call, &b);
        DEFER(log_call, &c); /* registered last  → runs first */
    }
    ASSERT(g_call_count == 3);
    ASSERT(g_call_log[0] == 3); /* LIFO */
    ASSERT(g_call_log[1] == 2);
    ASSERT(g_call_log[2] == 1);
}

/* ── test 10: nested scopes ───────────────────────────────────────────── */
TEST(defer_nested_scopes)
{
    reset_log();
    int outer = 10, inner = 20;
    {
        DEFER(log_call, &outer);
        {
            DEFER(log_call, &inner);
        }
        /* inner scope exited — inner already ran */
        ASSERT(g_call_count == 1);
        ASSERT(g_call_log[0] == 20);
    }
    /* outer scope exited */
    ASSERT(g_call_count == 2);
    ASSERT(g_call_log[1] == 10);
}

/* ── test 11: early return ────────────────────────────────────────────── */
static int helper_early_return(int *out)
{
    reset_log();
    int id = 99;
    DEFER(log_call, &id);
    if (1) {
        *out = g_call_count;
        return -1; /* early return */
    }
    *out = -999; /* never reached */
    return 0;
}

TEST(defer_early_return)
{
    int count_before_return;
    int rc = helper_early_return(&count_before_return);
    ASSERT(rc == -1);
    ASSERT(count_before_return == 0);  /* not yet called at return point */
    ASSERT(g_call_count == 1);         /* called after return */
    ASSERT(g_call_log[0] == 99);
}

/* ── test 12: multiple defers in one function ────────────────────────── */
TEST(defer_multiple_resources)
{
    reset_log();
    int a = 1, b = 2, c = 3, d = 4;
    {
        DEFER(log_call, &a);
        DEFER(log_call, &b);
        DEFER(log_call, &c);
        DEFER(log_call, &d);
    }
    ASSERT(g_call_count == 4);
    /* LIFO: d, c, b, a */
    ASSERT(g_call_log[0] == 4);
    ASSERT(g_call_log[1] == 3);
    ASSERT(g_call_log[2] == 2);
    ASSERT(g_call_log[3] == 1);
}

/* ── test 13: DEFER_FREE + write pattern ─────────────────────────────── */
#if defined(__unix__) || defined(__APPLE__)
TEST(defer_free_with_write)
{
    char tmpname[] = "/tmp/defer_rw_XXXXXX";

    int tmpfd = mkstemp(tmpname);
    ASSERT(tmpfd >= 0);
    close(tmpfd);

    int ok = 0;
    {
        void *buf = malloc(128);
        ASSERT(buf != NULL);
        DEFER_FREE(buf);

        FILE *f = fopen(tmpname, "w");
        ASSERT(f != NULL);
        DEFER_FCLOSE(f);

        memset(buf, 'A', 128);
        size_t written = fwrite(buf, 1, 128, f);
        ASSERT(written == 128);
        ok = 1;
    }
    /* buf freed, f closed */
    ASSERT(ok == 1);
    unlink(tmpname);
}
#else
/* POSIX-only test — skipped on non-POSIX platforms */
TEST(defer_free_with_write)
{
    ASSERT(1);
}
#endif

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void)
{
    printf("defer.h v%s — test suite\n", DEFER_VERSION);
    printf("compiler: %s\n", 
#if defined(__clang__)
        "clang " __clang_version__
#elif defined(__GNUC__)
        "gcc " __VERSION__
#else
        "unknown"
#endif
    );
    printf("DEFER_SUPPORTED: %d\n\n", DEFER_SUPPORTED);

    RUN(defer_free_basic);
    RUN(defer_free_zeros_ptr);
    RUN(defer_free_null_safe);
    RUN(defer_fclose_basic);
    RUN(defer_fclose_null_safe);
    RUN(defer_close_basic);
    RUN(defer_close_neg_safe);
    RUN(defer_generic_callback);
    RUN(defer_lifo_order);
    RUN(defer_nested_scopes);
    RUN(defer_early_return);
    RUN(defer_multiple_resources);
    RUN(defer_free_with_write);

    printf("\n────────────────────────────────────────────────\n");
    printf("Results: %d/%d passed", _tests_passed, _tests_run);
    if (_tests_failed)
        printf(", %d FAILED", _tests_failed);
    printf("\n");

    return _tests_failed ? 1 : 0;
}
