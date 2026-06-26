#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFER_ENABLE_FREE_HELPER
#define DEFER_ENABLE_STDIO_HELPER

#include "../defer.h"

#define BUF_SIZE 256

static size_t bounded_length(const char *text, size_t limit)
{
    size_t len = 0;
    while (len < limit && text[len] != '\0')
        ++len;
    return len;
}

static void uppercase_copy(char *dst, size_t dstsz, const char *src)
{
    size_t len = bounded_length(src, dstsz - 1);
    memcpy(dst, src, len);
    dst[len] = '\0';

    for (size_t i = 0; i < len; ++i) {
        if (dst[i] >= 'a' && dst[i] <= 'z')
            dst[i] = (char)(dst[i] - ('a' - 'A'));
    }
}

static int process_data(const char *input, char *output, size_t outsz)
{
    if (input == NULL || output == NULL || outsz == 0)
        return -1;

    char *tmp = (char *)malloc(BUF_SIZE);
    if (tmp == NULL)
        return -1;
    DEFER_FREE(tmp);

    char *work = (char *)malloc(BUF_SIZE);
    if (work == NULL)
        return -1;
    DEFER_FREE(work);

    uppercase_copy(tmp, BUF_SIZE, input);
    uppercase_copy(work, BUF_SIZE, tmp);
    uppercase_copy(output, outsz, work);
    return 0;
}

int main(void)
{
    char result[BUF_SIZE];

    printf("defer.h - memory example\n\n");
    if (process_data("hello from defer.h", result, sizeof(result)) == 0)
        printf("  Result: %s\n", result);

    printf("\nDone. No leaked buffers.\n");
    return 0;
}
