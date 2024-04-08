#include "hal/shutdown.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

static pthread_mutex_t lock;
static pthread_cond_t condition;
static bool* isRunning;

void shutdown_init(bool *running) {
    isRunning = running;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&condition, NULL);
}

void shutdown_wait() {
    pthread_mutex_lock(&lock);
    while (*isRunning) {
        pthread_cond_wait(&condition, &lock);
    }
    pthread_mutex_unlock(&lock);
}

void shutdown_signal() {
    printf("shutdown_signal(): Signalled shutdown...\n");
    pthread_mutex_lock(&lock);
    *isRunning = false;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&lock);
}

void shutdown_cleanup() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&condition);
}