#ifndef THE_SCHEDULER_H
#define THE_SCHEDULER_H

typedef struct thesched thesched;

struct the_job {
	void (*job)(void*);
	void *args;
};

thesched *the_sched_create(int thread_count);
void the_sched_do(thesched *s, struct the_job job);
void the_sched_wait(thesched *s);
int the_sched_destroy(thesched *s);

#endif // THE_SCHEDULER_H
