#include "../defer.h"

static int g_multi_b_hits;

static void multi_b_cleanup(void *ctx)
{
    int *value = (int *)ctx;
    g_multi_b_hits += *value;
}

int defer_multi_tu_b(void)
{
    int value = 5;
    {
        DEFER(multi_b_cleanup, &value);
    }
    return g_multi_b_hits;
}
