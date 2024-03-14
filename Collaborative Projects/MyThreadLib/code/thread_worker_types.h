#ifndef TW_TYPES_H
#define TW_TYPES_H

#ifdef MLFQ
#define NUM_QS 4
#else
#define NUM_QS 1
#endif

#include <ucontext.h>

typedef unsigned int worker_t;

typedef struct tcb {
	worker_t threadID;
    ucontext_t threadContext;
    worker_t status;
    void *retVal;
    struct waitingList *joiners;
} tcb;

typedef struct waitingList {
	tcb *thread;
	struct waitingList *next;
	struct waitingList *prev;
}waitingList;

#endif
