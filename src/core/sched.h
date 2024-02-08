#ifndef NYAS_SCHEDULER_H
#define NYAS_SCHEDULER_H

struct nyas_job {
	void (*job)(void*);
	void *args;
};

int nyas_sched_init(int thread_count, int queue_capacity);
void nyas_sched_do(struct nyas_job job);
void nyas_sched_wait(void);
int nyas_sched_destroy(void);

#endif // NYAS_SCHEDULER_H
