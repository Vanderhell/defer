#include "../../defer.h"

static void noop_cleanup(void *ctx)
{
    (void)ctx;
}

int main(void)
{
    goto inside;
    {
        int token = 1;
        DEFER(noop_cleanup, &token);
inside:
        return token;
    }
}
