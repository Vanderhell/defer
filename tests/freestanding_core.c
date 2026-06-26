#include "../defer.h"

static void freestanding_cleanup(void *ctx)
{
    (void)ctx;
}

void freestanding_entry(void)
{
    int token = 0;
    DEFER(freestanding_cleanup, &token);
}
