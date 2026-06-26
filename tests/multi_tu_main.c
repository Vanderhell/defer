#include <stdio.h>

int defer_multi_tu_a(void);
int defer_multi_tu_b(void);

int main(void)
{
    int a = defer_multi_tu_a();
    int b = defer_multi_tu_b();
    printf("%d %d\n", a, b);
    return (a == 3 && b == 5) ? 0 : 1;
}
