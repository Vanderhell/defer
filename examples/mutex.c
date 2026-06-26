#include <stdio.h>

#if defined(__has_include)
#  if __has_include(<pthread.h>)
#    define DEFER_EXAMPLE_HAVE_PTHREAD 1
#  endif
#endif

#if defined(DEFER_EXAMPLE_HAVE_PTHREAD)
#  define DEFER_WITH_PTHREAD
#  include <pthread.h>
#endif

#include "../defer.h"

#if defined(DEFER_EXAMPLE_HAVE_PTHREAD)

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_counter = 0;

static int increment_safe(int amount)
{
    int rc = pthread_mutex_lock(&g_mutex);
    if (rc != 0)
        return -1;

    DEFER_UNLOCK(&g_mutex);

    if (amount < 0)
        return -1;

    g_counter += amount;
    return g_counter;
}

int main(void)
{
    printf("defer.h - mutex example\n\n");

    printf("  increment(5)  = %d\n", increment_safe(5));
    printf("  increment(3)  = %d\n", increment_safe(3));
    printf("  increment(-1) = %d (rejected)\n", increment_safe(-1));
    printf("  increment(2)  = %d\n", increment_safe(2));
    printf("  final counter = %d\n", g_counter);

    printf("\nDone. Mutex is unlocked only after a successful lock.\n");
    return 0;
}

#else

int main(void)
{
    printf("defer.h - mutex example\n\n");
    printf("  pthread.h unavailable on this platform.\n");
    return 0;
}

#endif
