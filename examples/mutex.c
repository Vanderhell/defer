/*
 * examples/mutex.c — Mutex handling with defer.h
 *
 * Demonstrates: DEFER_UNLOCK with pthread_mutex.
 * Compile: gcc -std=c11 -o mutex examples/mutex.c -lpthread
 */

#define _POSIX_C_SOURCE 200809L
#define DEFER_WITH_PTHREAD
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../defer.h"

static pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
static int             g_counter = 0;

static int increment_safe(int amount)
{
    pthread_mutex_lock(&g_mtx);
    DEFER_UNLOCK(&g_mtx); /* unlocks on any return path */

    if (amount < 0)
        return -1; /* mutex unlocked automatically */

    g_counter += amount;
    return g_counter;
}

int main(void)
{
    printf("defer.h — mutex example\n\n");

    printf("  increment(5)  = %d\n", increment_safe(5));
    printf("  increment(3)  = %d\n", increment_safe(3));
    printf("  increment(-1) = %d (rejected)\n", increment_safe(-1));
    printf("  increment(2)  = %d\n", increment_safe(2));
    printf("  final counter = %d\n", g_counter);

    printf("\nDone. Mutex never leaked.\n");
    return 0;
}
