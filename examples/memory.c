#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFER_ENABLE_FREE_HELPER
#define DEFER_ENABLE_STDIO_HELPER

#include "../defer.h"

#define BUF_SIZE 256

static void uppercase_copy(char *dst, size_t dstsz, const char *src)
{
    size_t i = 0;

    if (dstsz == 0)
        return;

    while (i + 1u < dstsz && src[i] != '\0') {
        char ch = src[i];
        if (ch >= 'a' && ch <= 'z')
            ch = (char)(ch - ('a' - 'A'));
        dst[i] = ch;
        ++i;
    }

    dst[i] = '\0';
}

typedef struct buffer_state_s {
    char *ptr;
} buffer_state_t;

static void buffer_cleanup(void *ctx)
{
    buffer_state_t *state = (buffer_state_t *)ctx;
    if (state->ptr != NULL) {
        free(state->ptr);
        state->ptr = NULL;
    }
}

static int process_data(const char *input, char *output, size_t outsz)
{
    int rc = -1;
    buffer_state_t tmp_state = { NULL };
    buffer_state_t work_state = { NULL };

    if (input == NULL || output == NULL || outsz == 0)
        return -1;

    DEFER_NAMED(tmp_guard, buffer_cleanup, &tmp_state);
    DEFER_NAMED(work_guard, buffer_cleanup, &work_state);

    tmp_state.ptr = (char *)malloc(BUF_SIZE);
    if (tmp_state.ptr == NULL)
        goto cleanup;

    work_state.ptr = (char *)malloc(BUF_SIZE);
    if (work_state.ptr == NULL)
        goto cleanup;

    uppercase_copy(tmp_state.ptr, BUF_SIZE, input);
    uppercase_copy(work_state.ptr, BUF_SIZE, tmp_state.ptr);
    uppercase_copy(output, outsz, work_state.ptr);

    rc = 0;

cleanup:
    if (work_state.ptr != NULL) {
        free(work_state.ptr);
        work_state.ptr = NULL;
    }

    if (tmp_state.ptr != NULL) {
        free(tmp_state.ptr);
        tmp_state.ptr = NULL;
    }

    return rc;
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
