#include <stdio.h>
#include <stdlib.h>

#define DEFER_ENABLE_FREE_HELPER

#include "defer.h"

static int cleanup_hits;

static void count_cleanup(void *ctx)
{
    int *value = (int *)ctx;
    ++(*value);
}

int main(void)
{
    char *buffer = (char *)malloc(16);
    if (buffer == NULL)
        return 1;

    {
        DEFER_FREE(buffer);
    }

    {
        DEFER(count_cleanup, &cleanup_hits);
    }

    printf("%d\n", cleanup_hits);
    return cleanup_hits == 1 ? 0 : 1;
}
