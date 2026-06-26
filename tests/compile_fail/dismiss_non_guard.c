#include "../../defer.h"

int main(void)
{
    int not_a_guard = 0;
    DEFER_DISMISS(not_a_guard);
    return 0;
}
