#include "../../defer.h"

static int bad_cleanup(void *ctx)
{
    (void)ctx;
    return 0;
}

int main(void)
{
    DEFER(bad_cleanup, (void *)0);
    return 0;
}
