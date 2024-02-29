#ifndef NYAS_SCHEDULER_H
#define NYAS_SCHEDULER_H

typedef struct nysched nysched;

struct nyas_job {
	void (*job)(void*);
	void *args;
};

nysched *nyas_sched_create(int thread_count);
void nyas_sched_do(nysched *s, struct nyas_job job);
void nyas_sched_wait(nysched *s);
int nyas_sched_destroy(nysched *s);

#endif // NYAS_SCHEDULER_H
