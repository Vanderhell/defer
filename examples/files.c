#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFER_ENABLE_STDIO_HELPER

#include "../defer.h"

typedef struct file_state_s {
    FILE *fp;
} file_state_t;

static void file_close_cleanup(void *ctx)
{
    file_state_t *state = (file_state_t *)ctx;
    if (state->fp != NULL)
        fclose(state->fp);
}

static int count_lines(FILE *fp)
{
    int lines = 0;
    int ch;

    rewind(fp);
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n')
            ++lines;
    }

    if (ferror(fp))
        return -1;

    return lines;
}

static int copy_stream(FILE *src, FILE *dst)
{
    char buffer[128];

    rewind(src);
    for (;;) {
        size_t nread = fread(buffer, 1u, sizeof(buffer), src);
        if (nread > 0) {
            size_t nwritten = fwrite(buffer, 1u, nread, dst);
            if (nwritten != nread)
                return -1;
        }

        if (nread < sizeof(buffer))
            break;
    }

    if (ferror(src) || fflush(dst) != 0)
        return -1;

    return 0;
}

static int copy_and_verify(void)
{
    FILE *src = tmpfile();
    if (src == NULL)
        return -1;

    DEFER_FCLOSE(src);

    FILE *dst = tmpfile();
    if (dst == NULL)
        return -1;

    file_state_t dst_state = { dst };
    DEFER_NAMED(dst_guard, file_close_cleanup, &dst_state);

    if (fputs("line one\nline two\nline three\n", src) == EOF)
        return -1;

    if (copy_stream(src, dst) != 0)
        return -1;

    rewind(dst);
    if (count_lines(dst) != 3)
        return -1;

    {
        int close_rc = fclose(dst);
        dst_state.fp = NULL;
        DEFER_DISMISS(dst_guard);
        if (close_rc != 0)
            return -1;
    }

    return count_lines(src);
}

int main(void)
{
    int lines = copy_and_verify();

    printf("defer.h - files example\n\n");
    if (lines >= 0)
        printf("  Lines copied: %d\n", lines);

    printf("\nDone. Resources are closed explicitly when close errors matter.\n");
    return lines >= 0 ? 0 : 1;
}
