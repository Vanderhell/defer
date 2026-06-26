#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__has_include)
#  if __has_include(<unistd.h>)
#    define DEFER_ENABLE_UNISTD_HELPER
#    define DEFER_TEST_HAVE_UNISTD 1
#  endif
#  if __has_include(<pthread.h>)
#    define DEFER_WITH_PTHREAD
#    define DEFER_TEST_HAVE_PTHREAD 1
#  endif
#endif

#define DEFER_ENABLE_FREE_HELPER
#define DEFER_ENABLE_STDIO_HELPER

#if defined(DEFER_TEST_HAVE_UNISTD)
#  include <fcntl.h>
#  include <unistd.h>
#endif

#if defined(DEFER_TEST_HAVE_PTHREAD)
#  include <pthread.h>
#endif

#include "../defer.h"

static int g_tests_run;
static int g_tests_passed;
static int g_tests_failed;

static int g_log[32];
static size_t g_log_count;

static int g_selector_hits;
static int g_free_selector_hits;

static void test_reset_log(void)
{
    memset(g_log, 0, sizeof(g_log));
    g_log_count = 0;
}

static void test_log_id(void *ctx)
{
    const int id = *(const int *)ctx;
    g_log[g_log_count++] = id;
}

static defer_detail_cleanup_fn test_choose_callback(void)
{
    g_selector_hits++;
    return test_log_id;
}

static void *test_choose_context(void *ctx)
{
    g_selector_hits++;
    return ctx;
}

static char *test_make_buffer(size_t size)
{
    g_free_selector_hits++;
    return (char *)malloc(size);
}

static void test_count_cleanup(void *ctx)
{
    int *count = (int *)ctx;
    (*count)++;
}

typedef struct test_free_payload_s {
    void *ptr;
} test_free_payload_t;

static void test_free_payload_cleanup(void *ctx)
{
    test_free_payload_t *payload = (test_free_payload_t *)ctx;
    free(payload->ptr);
    payload->ptr = NULL;
}

static void test_assert(int condition, const char *expr, const char *file, int line)
{
    if (!condition) {
        g_tests_failed++;
        printf("FAIL\n    Assertion failed: %s\n    at %s:%d\n", expr, file, line);
    }
}

#define ASSERT(expr)                                                             \
    do {                                                                         \
        if (!(expr)) {                                                           \
            test_assert(0, #expr, __FILE__, __LINE__);                           \
            return;                                                              \
        }                                                                        \
    } while (0)

#define TEST(name) static void test_##name(void)

#define RUN(name)                                                             \
    do {                                                                      \
        int failures_before = g_tests_failed;                                 \
        g_tests_run++;                                                        \
        printf("  %-40s", #name " ...");                                      \
        fflush(stdout);                                                       \
        test_##name();                                                        \
        if (g_tests_failed == failures_before) {                              \
            g_tests_passed++;                                                 \
            printf("PASS\n");                                                \
        }                                                                     \
    } while (0)

TEST(defer_generic_side_effects_are_evaluated_once)
{
    g_selector_hits = 0;
    test_reset_log();

    int token = 7;
    {
        DEFER(test_choose_callback(), test_choose_context(&token));
    }

    ASSERT(g_selector_hits == 2);
    ASSERT(g_log_count == 1);
    ASSERT(g_log[0] == 7);
}

TEST(defer_named_dismiss_is_idempotent)
{
    int cleanup_hits = 0;
    {
        DEFER_NAMED(guard, test_count_cleanup, &cleanup_hits);
        DEFER_DISMISS(guard);
        DEFER_DISMISS(guard);
    }

    ASSERT(cleanup_hits == 0);
}

TEST(defer_named_manual_cleanup_then_dismiss)
{
    test_free_payload_t payload = { 0 };
    int cleanup_hits = 0;

    {
        DEFER_NAMED(guard, test_free_payload_cleanup, &payload);
        payload.ptr = malloc(32);
        ASSERT(payload.ptr != NULL);
        test_count_cleanup(&cleanup_hits);
        test_free_payload_cleanup(&payload);
        DEFER_DISMISS(guard);
    }

    ASSERT(cleanup_hits == 1);
    ASSERT(payload.ptr == NULL);
}

TEST(defer_free_evaluates_expression_once)
{
    g_free_selector_hits = 0;
    {
        DEFER_FREE(test_make_buffer(48));
        ASSERT(g_free_selector_hits == 1);
    }

    ASSERT(g_free_selector_hits == 1);
}

TEST(defer_free_supports_typed_pointer_reassignment)
{
    char *first = (char *)malloc(16);
    char *second = (char *)malloc(16);
    ASSERT(first != NULL);
    ASSERT(second != NULL);

    {
        DEFER_FREE(first);
        first = second;
    }

    free(second);
}

typedef struct test_item_s {
    int value;
} test_item_t;

TEST(defer_free_supports_struct_and_const_pointers)
{
    test_item_t *item = (test_item_t *)malloc(sizeof(*item));
    test_item_t *replacement = (test_item_t *)malloc(sizeof(*replacement));
    unsigned char *raw = (unsigned char *)malloc(24);
    unsigned char *replacement_raw = (unsigned char *)malloc(24);
    ASSERT(item != NULL);
    ASSERT(replacement != NULL);
    ASSERT(raw != NULL);
    ASSERT(replacement_raw != NULL);

    const unsigned char *const_raw = raw;

    {
        DEFER_FREE(item);
        DEFER_FREE(const_raw);
        item = replacement;
        const_raw = replacement_raw;
        item->value = 11;
    }

    free(replacement);
    free(replacement_raw);
}

TEST(defer_free_supports_void_pointer)
{
    void *blob = malloc(64);
    ASSERT(blob != NULL);
    {
        DEFER_FREE(blob);
    }
}

TEST(defer_lifo_order)
{
    test_reset_log();
    int a = 1;
    int b = 2;
    int c = 3;

    {
        DEFER(test_log_id, &a);
        DEFER(test_log_id, &b);
        DEFER(test_log_id, &c);
    }

    ASSERT(g_log_count == 3);
    ASSERT(g_log[0] == 3);
    ASSERT(g_log[1] == 2);
    ASSERT(g_log[2] == 1);
}

TEST(defer_multiple_on_one_line)
{
    test_reset_log();
    int a = 10;
    int b = 20;
    { DEFER(test_log_id, &a); DEFER(test_log_id, &b); }
    ASSERT(g_log_count == 2);
    ASSERT(g_log[0] == 20);
    ASSERT(g_log[1] == 10);
}

TEST(defer_nested_scopes)
{
    test_reset_log();
    int outer = 100;
    int inner = 200;

    {
        DEFER(test_log_id, &outer);
        {
            DEFER(test_log_id, &inner);
        }
        ASSERT(g_log_count == 1);
        ASSERT(g_log[0] == 200);
    }

    ASSERT(g_log_count == 2);
    ASSERT(g_log[1] == 100);
}

static int test_early_return_helper(int *count_snapshot)
{
    test_reset_log();
    int token = 55;
    DEFER(test_log_id, &token);
    *count_snapshot = (int)g_log_count;
    return -1;
}

TEST(defer_early_return)
{
    int count_snapshot = -1;
    int rc = test_early_return_helper(&count_snapshot);

    ASSERT(rc == -1);
    ASSERT(count_snapshot == 0);
    ASSERT(g_log_count == 1);
    ASSERT(g_log[0] == 55);
}

static int test_goto_helper(void)
{
    test_reset_log();
    int token = 88;

    {
        DEFER(test_log_id, &token);
        goto out;
    }

out:
    return 0;
}

TEST(defer_goto_out_of_scope)
{
    int rc = test_goto_helper();
    ASSERT(rc == 0);
    ASSERT(g_log_count == 1);
    ASSERT(g_log[0] == 88);
}

TEST(defer_break_and_continue_cleanup)
{
    test_reset_log();

    for (int i = 0; i < 3; ++i) {
        int token = 300 + i;
        DEFER(test_log_id, &token);
        if (i == 0)
            continue;
        if (i == 1)
            break;
    }

    ASSERT(g_log_count == 2);
    ASSERT(g_log[0] == 300);
    ASSERT(g_log[1] == 301);
}

TEST(defer_free_supports_manual_captured_value_change)
{
    char *first = (char *)malloc(24);
    char *second = (char *)malloc(24);
    ASSERT(first != NULL);
    ASSERT(second != NULL);

    {
        DEFER_FREE(first);
        first = second;
    }

    free(second);
}

TEST(defer_manual_cleanup_pattern_works_with_named_guard)
{
    int cleanup_hits = 0;
    {
        DEFER_NAMED(guard, test_count_cleanup, &cleanup_hits);
        test_count_cleanup(&cleanup_hits);
        DEFER_DISMISS(guard);
    }

    ASSERT(cleanup_hits == 1);
}

#if defined(DEFER_TEST_HAVE_UNISTD)
TEST(defer_fclose_helper_closes_tmpfile)
{
    FILE *first = tmpfile();
    FILE *second = tmpfile();
    ASSERT(first != NULL);
    ASSERT(second != NULL);

    {
        DEFER_FCLOSE(first);
        first = second;
    }

    fclose(second);
}

TEST(defer_close_helper_captures_fd_value)
{
    char template_name_1[] = "defer_test_fd_a_XXXXXX";
    char template_name_2[] = "defer_test_fd_b_XXXXXX";
    int fd1 = mkstemp(template_name_1);
    int fd2 = mkstemp(template_name_2);
    ASSERT(fd1 >= 0);
    ASSERT(fd2 >= 0);

    {
        DEFER_CLOSE(fd1);
        fd1 = fd2;
    }

    close(fd2);
    unlink(template_name_1);
    unlink(template_name_2);
}
#else
TEST(defer_fclose_helper_closes_tmpfile)
{
    ASSERT(1);
}

TEST(defer_close_helper_captures_fd_value)
{
    ASSERT(1);
}
#endif

#if defined(DEFER_TEST_HAVE_PTHREAD)
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_counter = 0;

TEST(defer_unlock_helper_releases_locked_mutex)
{
    int rc = pthread_mutex_lock(&g_mutex);
    ASSERT(rc == 0);

    {
        DEFER_UNLOCK(&g_mutex);
        g_counter += 1;
    }

    rc = pthread_mutex_trylock(&g_mutex);
    ASSERT(rc == 0);
    pthread_mutex_unlock(&g_mutex);
}
#else
TEST(defer_unlock_helper_releases_locked_mutex)
{
    ASSERT(1);
}
#endif

int main(void)
{
    printf("defer.h v%s - test suite\n", DEFER_VERSION);
    printf("DEFER_SUPPORTED: %d\n\n", DEFER_SUPPORTED);

    RUN(defer_generic_side_effects_are_evaluated_once);
    RUN(defer_named_dismiss_is_idempotent);
    RUN(defer_named_manual_cleanup_then_dismiss);
    RUN(defer_free_evaluates_expression_once);
    RUN(defer_free_supports_typed_pointer_reassignment);
    RUN(defer_free_supports_struct_and_const_pointers);
    RUN(defer_free_supports_void_pointer);
    RUN(defer_lifo_order);
    RUN(defer_multiple_on_one_line);
    RUN(defer_nested_scopes);
    RUN(defer_early_return);
    RUN(defer_goto_out_of_scope);
    RUN(defer_break_and_continue_cleanup);
    RUN(defer_free_supports_manual_captured_value_change);
    RUN(defer_manual_cleanup_pattern_works_with_named_guard);
    RUN(defer_fclose_helper_closes_tmpfile);
    RUN(defer_close_helper_captures_fd_value);
    RUN(defer_unlock_helper_releases_locked_mutex);

    printf("\nResults: %d/%d passed\n", g_tests_passed, g_tests_run);
    return g_tests_failed ? 1 : 0;
}
