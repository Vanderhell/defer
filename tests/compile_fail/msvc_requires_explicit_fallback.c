#if !defined(_MSC_VER)
#  error "MSVC-only contract test"
#endif

#include "../../defer.h"

int main(void)
{
    DEFER((void (*)(void *))0, (void *)0);
    return 0;
}
