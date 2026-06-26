#include <stdio.h>

#if defined(__has_include)
#  if __has_include(<pthread.h>)
#    define DEFER_THREAD_TEST_HAVE_PTHREAD 1
#  endif
#endif

#if defined(DEFER_THREAD_TEST_HAVE_PTHREAD)
#  define DEFER_WITH_PTHREAD
#  include <pthread.h>
#endif

#include "../defer.h"

#if defined(DEFER_THREAD_TEST_HAVE_PTHREAD)

static pthread_mutex_t g_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_thread_hits = 0;

static void thread_cleanup(void *ctx)
{
    int *hits = (int *)ctx;
    ++(*hits);
}

static void *thread_entry(void *arg)
{
    int *hits = (int *)arg;
    int rc = pthread_mutex_lock(&g_thread_mutex);
    if (rc != 0)
        return (void *)1;

    DEFER_UNLOCK(&g_thread_mutex);
    {
        DEFER(thread_cleanup, hits);
    }

    return 0;
}

int main(void)
{
    pthread_t left;
    pthread_t right;
    int rc = pthread_create(&left, 0, thread_entry, &g_thread_hits);
    if (rc != 0)
    {
        fprintf(stderr, "FAIL: pthread_create(left) rc=%d\n", rc);
        return 1;
    }
    rc = pthread_create(&right, 0, thread_entry, &g_thread_hits);
    if (rc != 0)
    {
        fprintf(stderr, "FAIL: pthread_create(right) rc=%d\n", rc);
        return 1;
    }

    rc = pthread_join(left, 0);
    if (rc != 0)
    {
        fprintf(stderr, "FAIL: pthread_join(left) rc=%d\n", rc);
        return 1;
    }
    rc = pthread_join(right, 0);
    if (rc != 0)
    {
        fprintf(stderr, "FAIL: pthread_join(right) rc=%d\n", rc);
        return 1;
    }

    if (g_thread_hits != 2)
    {
        fprintf(stderr, "FAIL: expected 2 thread cleanups, got %d\n", g_thread_hits);
        return 1;
    }

    printf("PASS: thread cleanups = %d\n", g_thread_hits);
    return 0;
}

#else

int main(void)
{
    puts("pthread unavailable");
    return 0;
}

#endif
