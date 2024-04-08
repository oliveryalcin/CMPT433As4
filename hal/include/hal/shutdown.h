#ifndef _SHUTDOWN_H_
#define _SHUTDOWN_H_

#include <stdbool.h>

void shutdown_init(bool *running);
void shutdown_wait();
void shutdown_signal();
void shutdown_cleanup();

#endif // _SHUTDOWN_H_