#include "thread-worker.h"
#include "thread_worker_types.h"

#define STACK_SIZE 16 * 1024
#define QUANTUM 10 * 1000
#define READY 0
#define SCHEDULED 1
#define BLOCKED 2
#define TERMINATED 3
#define JOINED 4


// INITIALIZE ALL YOUR OTHER VARIABLES HERE
int init_scheduler_done = 0;
worker_t currentNewTID = 0;
worker_t currentNewMutexID = 0;

ucontext_t schedContext;
tcb* mainThread = NULL;
waitingList* first = NULL;
tcb* currentTCB = NULL;

static void enableRotation()
{
	struct itimerval timer = {.it_interval = {0, 0},.it_value = {0, QUANTUM}};
	setitimer(ITIMER_PROF, &timer, NULL);
}

static void disableRotation()
{
	struct itimerval timer = {.it_interval = {0, 0},.it_value = {0, 0}};
	setitimer(ITIMER_PROF, &timer, NULL);
}

static void enqueue(tcb *thread)
{
	waitingList *newNode = (waitingList*)malloc(sizeof(waitingList));
	
	newNode->thread = thread;
	newNode->next = newNode;
	newNode->prev = newNode;

	if (!first)
		first = newNode;
    
    newNode->next = first;
    newNode->prev = first->prev;
    first->prev->next = newNode;
    first->prev = newNode;
}

static tcb *nextTCBToRun()
{
	waitingList **ptr = &first;
	waitingList *head = *ptr;
	tcb *DQd;

	if (!*ptr)
		exit(-1);
	while ((*ptr)->thread->status != READY) {
		if ((*ptr)->thread->status == JOINED) {
			waitingList *toFree = *ptr;
			toFree->prev->next = toFree->next;
            toFree->next->prev = toFree->prev;
            
			*ptr = (*ptr)->next;
			free(toFree->thread->threadContext.uc_stack.ss_sp);
			free(toFree->thread);
			free(toFree);
			if (toFree == head) {
				head = (*ptr);
				continue;
			}
		} else {
			*ptr = (*ptr)->next;
		}
	}
	DQd = (*ptr)->thread;
	*ptr = (*ptr)->next;
	return DQd;
}

static void rotation(int signum)
{
	disableRotation();
	swapcontext(&currentTCB->threadContext, &schedContext);
}

static void initializeSched()
{
	struct sigaction sa;
	int i;

	mainThread = (tcb*)malloc(sizeof(tcb));

    getcontext(&mainThread->threadContext);
	getcontext(&schedContext);

	schedContext.uc_link = NULL;
	schedContext.uc_stack.ss_flags = 0;
	schedContext.uc_stack.ss_size = STACK_SIZE;
	schedContext.uc_stack.ss_sp = malloc(STACK_SIZE);
	makecontext(&schedContext, schedule, 0);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = rotation;
	sigaction(SIGPROF, &sa, NULL);

	mainThread->threadID = currentNewTID++;
	mainThread->status = SCHEDULED;
	mainThread->joiners = NULL;
	currentTCB = mainThread;

	enqueue(mainThread);
}

static tcb *findTCBFromTID(worker_t threadID)
{
	waitingList *ptr = first;
	while(first) {
		ptr = ptr->next;
		if (ptr->thread->threadID == threadID)
			return ptr->thread;
		    if (ptr == first)
			    break;
	}
	
	return NULL;
}

static void waitlist(waitingList **list, tcb *waiter)
{
	waitingList *new_waiter = (waitingList*)malloc(sizeof(waitingList));
	new_waiter->thread = waiter;
	new_waiter->next = *list;
	*list = new_waiter;
}

static void releaseFromWaitlist(waitingList *list)
{
	waitingList *ptr;

	while(list){
		ptr = list->next;
		list->thread->status = READY;
		free(list);
		list = ptr;
	}
}

int worker_create(worker_t *thread, pthread_attr_t *attr, void *(*function)(void*), void *arg)
{
	tcb *new_tcb;

	if (!init_scheduler_done) {
        init_scheduler_done = 1;
		initializeSched();
		enableRotation();
	}

	new_tcb = (tcb*)malloc(sizeof(tcb));

	getcontext(&new_tcb->threadContext);

	new_tcb->threadContext.uc_link = &currentTCB->threadContext;
	new_tcb->threadContext.uc_stack.ss_flags = 0;
	new_tcb->threadContext.uc_stack.ss_size = STACK_SIZE;
	new_tcb->threadContext.uc_stack.ss_sp = malloc(STACK_SIZE);
	makecontext(&new_tcb->threadContext, (void*)function, 1,arg);

	new_tcb->status = READY;
	new_tcb->joiners = NULL;
	new_tcb->threadID = currentNewTID;
	*thread = currentNewTID;
	currentNewTID++;
	enqueue(new_tcb);
	return 0;
}

int worker_yield()
{
	disableRotation();
	return swapcontext(&currentTCB->threadContext, &schedContext);
}

void worker_exit(void *value_ptr)
{
	disableRotation();
	currentTCB->retVal = value_ptr;
	currentTCB->status = TERMINATED;
	releaseFromWaitlist(currentTCB->joiners);
	currentTCB->joiners = NULL;
	setcontext(&schedContext);
}

int worker_join(worker_t thread, void **value_ptr)
{
	tcb *join_thread = findTCBFromTID(thread);

	if (join_thread->status != TERMINATED) {
		disableRotation();
		waitlist(&join_thread->joiners, currentTCB);
		currentTCB->status = BLOCKED;
		swapcontext(&currentTCB->threadContext, &schedContext);
	}
	if (value_ptr!=NULL)
		*value_ptr = join_thread->retVal;
	join_thread->status = JOINED;
	return 0;
}

int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
	mutex->mutexID = currentNewMutexID;
	currentNewMutexID++;
	mutex->wait_list = NULL;
	mutex->current = NULL;
	return 0;
}

int worker_mutex_lock(worker_mutex_t *mutex)
{
	if (!mutex)
		return -1;
	while ((tcb *volatile) mutex->current) {
		waitingList *waiting_thread;
		disableRotation();
		currentTCB->status = BLOCKED;
		waiting_thread = (waitingList*)malloc(sizeof(waitingList));
		waiting_thread->thread = currentTCB;
		waiting_thread->next = mutex->wait_list;
		mutex->wait_list = waiting_thread;
		swapcontext(&currentTCB->threadContext, &schedContext);
	}
	mutex->current = currentTCB;
	return 0;
}

int worker_mutex_unlock(worker_mutex_t *mutex)
{
	sigset_t x;

	if (!mutex)
		return -1;
	sigemptyset(&x);
	sigaddset(&x, SIGPROF);
	sigprocmask(SIG_BLOCK, &x, NULL);
	mutex->current = NULL;
	releaseFromWaitlist(mutex->wait_list);
	mutex->wait_list = NULL;
	sigprocmask(SIG_UNBLOCK, &x, NULL);
	return 0;
}

int worker_mutex_destroy(worker_mutex_t *mutex)
{
	if (!mutex)
		return -1;
	if (mutex->current)
		releaseFromWaitlist(mutex->wait_list);
	mutex->current = NULL;
	return 0;
}

static void sched_rr()
{
	tcb *next_thread, *running = currentTCB;
	if (running->status == SCHEDULED)
		running->status = READY;
	next_thread = nextTCBToRun();
	next_thread->status = SCHEDULED;
	currentTCB = next_thread;
	enableRotation();
	swapcontext(&schedContext, &next_thread->threadContext);
}

static void sched_mlfq()
{}

static void schedule()
{
	for (;;) {
#ifndef MLFQ
		sched_rr();
#else
		sched_mlfq();
#endif
	}

}
