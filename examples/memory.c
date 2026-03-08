/*
 * examples/memory.c — Memory management with defer.h
 *
 * Demonstrates: DEFER_FREE with multiple allocations and early returns.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../defer.h"

#define BUF_SIZE 256

/*
 * Simulates a function that allocates multiple buffers and
 * may fail partway through. No goto, no manual cleanup chains.
 */
static int process_data(const char *input, char *output, size_t outsz)
{
    /* step 1 — working buffer */
    void *tmp = malloc(BUF_SIZE);
    if (!tmp) return -1;
    DEFER_FREE(tmp);

    /* step 2 — secondary buffer for transform */
    void *work = malloc(BUF_SIZE);
    if (!work) return -1;        /* tmp freed automatically */
    DEFER_FREE(work);

    /* simulate work */
    strncpy(tmp,  input, BUF_SIZE - 1);
    strncpy(work, tmp,   BUF_SIZE - 1);

    /* uppercase transform */
    for (char *p = work; *p; p++)
        if (*p >= 'a' && *p <= 'z') *p -= 32;

    strncpy(output, work, outsz - 1);
    output[outsz - 1] = '\0';

    return 0;
    /* work freed, tmp freed — LIFO */
}

int main(void)
{
    printf("defer.h — memory example\n\n");

    char result[BUF_SIZE];
    if (process_data("hello from defer.h", result, sizeof(result)) == 0)
        printf("  Result: %s\n", result);

    printf("\nDone. No leaked buffers.\n");
    return 0;
}
