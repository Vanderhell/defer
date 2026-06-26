#define DEFER_ENABLE_FREE_HELPER

#include <stdlib.h>

#include "../defer.h"

static int g_multi_a_hits;

static void multi_a_cleanup(void *ctx)
{
    int *value = (int *)ctx;
    g_multi_a_hits += *value;
}

int defer_multi_tu_a(void)
{
    int value = 3;
    {
        DEFER(multi_a_cleanup, &value);
    }
    return g_multi_a_hits;
}
