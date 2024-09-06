#include "core/common.h"
#include "core/io.h"
#include "core/mem.h"
#include "sched.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct the_job job;
THE_DECL_ARR(job);
THE_IMPL_ARR(job);
THE_DECL_STACK(job);
THE_IMPL_STACK(job);

typedef pthread_t thread;
THE_DECL_ARR(thread);
THE_IMPL_ARR(thread);

enum sched_states {
	SCHED_UNINIT = 0,
	SCHED_RUNNING,
	SCHED_CLOSING,
	SCHED_CLOSED
};

struct thesched {
	struct thesched *next;
	struct thestack_job queue;
	struct thearr_thread *threads;
	pthread_mutex_t mtx;
	pthread_cond_t cond;
	int waiting;
	enum sched_states state;
};

static thesched *scheds = NULL;
static pthread_mutex_t scheds_mtx;
static bool initialized = false;

static void *
the__worker(void *data)
{
	thesched *s = data;
	while (1) {
		++s->waiting;
		pthread_mutex_lock(&s->mtx);
		while (!(s->queue.tail)) {
			pthread_cond_wait(&s->cond, &s->mtx);
			if (s->state == SCHED_CLOSING) {
				--s->waiting;
				pthread_mutex_unlock(&s->mtx);
				goto exit_worker;
			}
		}

		--s->waiting;
		struct the_job job = *(thestack_job_pop(&s->queue));
		pthread_mutex_unlock(&s->mtx);
		(*(job.job))(job.args);
	}

exit_worker:
	return NULL;
}

thesched *
the_sched_create(int thread_count)
{
	if (!initialized) {
		pthread_mutex_init(&scheds_mtx, NULL);
		initialized = true;
	}

	thesched *s = THE_ALLOC(sizeof(struct thesched));
	s->waiting = 0;
	s->threads = NULL;
	s->queue = (struct thestack_job){.buf = NULL, .tail = 0};

	pthread_mutex_init(&s->mtx, NULL);
	pthread_cond_init(&s->cond, NULL);

	for (int i = 0; i < thread_count; ++i) {
		if (pthread_create(thearr_thread_push(&s->threads), NULL, the__worker, s)) {
			THE_LOG_ERR("Thread creation error.");
			return NULL;
		}
	}

	s->state = SCHED_RUNNING;
	pthread_mutex_lock(&scheds_mtx);
	s->next = scheds;
	scheds = s;
	pthread_mutex_unlock(&scheds_mtx);
	return s;
}

void
the_sched_do(thesched *s, struct the_job job)
{
	if (!s->threads->count) {
		(*(job.job))(job.args);
		return;
	}
	pthread_mutex_lock(&s->mtx);
	struct the_job *next = thestack_job_push(&s->queue);
	*next = job;
	pthread_cond_signal(&s->cond);
	pthread_mutex_unlock(&s->mtx);
}

void
the_sched_wait(thesched *s)
{
	if (!s || !s->threads || !s->threads->count) {
		return;
	}

	while (s->queue.tail || s->waiting != s->threads->count) {
		pthread_cond_broadcast(&s->cond);
		nanosleep((const struct timespec[]){ { 0, 50000000L } }, NULL);
	}
}

int
the_sched_destroy(thesched *s)
{
	pthread_mutex_lock(&s->mtx);
	s->state = SCHED_CLOSING;
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mtx);
	for (int i = 0; i < s->threads->count; ++i) {
		pthread_join(s->threads->at[i], NULL);
	}

	thearr_thread_release(s->threads);
	thearr_job_release(s->queue.buf);
	s->threads = NULL;
	s->queue = (struct thestack_job){.buf = NULL, .tail = 0};
	pthread_mutex_lock(&s->mtx);
	pthread_mutex_unlock(&s->mtx);
	pthread_mutex_destroy(&s->mtx);
	pthread_cond_destroy(&s->cond);
	s->state = SCHED_CLOSED;

	pthread_mutex_lock(&scheds_mtx);
	if (s == scheds) {
		scheds = scheds->next;
	} else {
		thesched *i = scheds;
		while (i) {
			if (s == i->next) {
				i->next = s->next;
				break;
			}
			i = i->next;
		}

		if (!i) {
			/* That scheduler does not exist in the schedulers list */
			pthread_mutex_unlock(&scheds_mtx);
			return THE_ERR_INVALID_PTR;
		}
	}

	pthread_mutex_unlock(&scheds_mtx);
	THE_FREE(s);
	return THE_OK;
}
