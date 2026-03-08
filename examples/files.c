/*
 * examples/files.c — File handling with defer.h
 *
 * Demonstrates: DEFER_FCLOSE, DEFER_FREE, DEFER_CLOSE
 *
 * Before defer.h — classic C with goto cleanup:
 *
 *   int process(const char *path) {
 *       FILE *f = fopen(path, "r");
 *       if (!f) return -1;
 *       void *buf = malloc(512);
 *       if (!buf) { fclose(f); return -1; }
 *       int rc = do_work(f, buf);
 *       free(buf);
 *       fclose(f);
 *       return rc;
 *   }
 *
 * After defer.h:
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../defer.h"

static int count_lines(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    DEFER_FCLOSE(f);

    char *line = NULL;
    size_t cap  = 0;
    int    lines = 0;

    while (getline(&line, &cap, f) != -1)
        lines++;

    free(line); /* getline buffer — manual free (not allocated via our pattern) */
    return lines;
}

static int copy_file(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    DEFER_FCLOSE(in);

    FILE *out = fopen(dst, "wb");
    if (!out) return -1;
    DEFER_FCLOSE(out);

    void *buf = malloc(4096);
    if (!buf) return -1;
    DEFER_FREE(buf);

    size_t n;
    while ((n = fread(buf, 1, 4096, in)) > 0)
        fwrite(buf, 1, n, out);

    printf("  Copied '%s' -> '%s'\n", src, dst);
    return 0;
    /* buf freed, out closed, in closed — automatically, in LIFO order */
}

int main(void)
{
    /* create a temp source file */
    FILE *tmp = fopen("/tmp/defer_example_src.txt", "w");
    if (!tmp) { perror("fopen"); return 1; }
    fprintf(tmp, "line one\nline two\nline three\n");
    fclose(tmp);

    printf("defer.h — files example\n\n");

    int n = count_lines("/tmp/defer_example_src.txt");
    printf("  Lines: %d\n", n);

    copy_file("/tmp/defer_example_src.txt", "/tmp/defer_example_dst.txt");

    remove("/tmp/defer_example_src.txt");
    remove("/tmp/defer_example_dst.txt");

    printf("\nDone. No goto cleanup, no manual fclose/free.\n");
    return 0;
}
