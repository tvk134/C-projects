#ifndef MTX_TYPES_H
#define MTX_TYPES_H

#include "thread_worker_types.h"

/* mutex struct definition */
typedef struct worker_mutex_t {
	tcb *current;
	waitingList *wait_list;
	worker_t mutexID;
} worker_mutex_t;

#endif
