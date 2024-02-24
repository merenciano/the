#include "core/nyas_core.h"
#include "core/io.h"
#include "core/mem.h"
#include "sched.h"
#include "utils/array.h"
#include <pthread.h>

enum sched_states {
	SCHED_UNINIT = 0,
	SCHED_RUNNING,
	SCHED_CLOSING,
	SCHED_CLOSED
};

struct nysched {
	struct nysched *next;
	struct nyas_job *queue;
	pthread_t *threads;
	pthread_mutex_t mtx;
	pthread_cond_t cond;
	int waiting;
	enum sched_states state;
};

static nysched *scheds = NULL;
static pthread_mutex_t scheds_mtx;
static bool initialized = false;

static void *
nyas__worker(void *data)
{
	nysched *s = data;
	while (1) {
		++s->waiting;
		pthread_mutex_lock(&s->mtx);
		while (!nyas_arr_count(s->queue)) {
			pthread_cond_wait(&s->cond, &s->mtx);
			if (s->state == SCHED_CLOSING) {
				--s->waiting;
				pthread_mutex_unlock(&s->mtx);
				goto exit_worker;
			}
		}

		--s->waiting;
		struct nyas_job job = *nyas_arr_last(s->queue);
		nyas_arr_pop(s->queue);
		pthread_mutex_unlock(&s->mtx);
		(*(job.job))(job.args);
	}

exit_worker:
	return NULL;
}

nysched *
nyas_sched_create(int thread_count, int queue_capacity)
{
	if (!initialized) {
		pthread_mutex_init(&scheds_mtx, NULL);
		initialized = true;
	}

	nysched *s = NYAS_ALLOC(sizeof(struct nysched));
	s->waiting = 0;
	s->threads = nyas_arr_create(pthread_t, thread_count);
	s->queue = nyas_arr_create(struct nyas_job, queue_capacity);

	pthread_mutex_init(&s->mtx, NULL);
	pthread_cond_init(&s->cond, NULL);

	nyas_arr_push(s->threads, thread_count);
	for (int i = 0; i < nyas_arr_count(s->threads); ++i) {
		if (pthread_create(s->threads + i, NULL, nyas__worker, s)) {
			NYAS_LOG_ERR("Thread creation error.");
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
nyas_sched_do(nysched *s, struct nyas_job job)
{
	if (!nyas_arr_count(s->threads)) {
		(*(job.job))(job.args);
		return;
	}
	pthread_mutex_lock(&s->mtx);
	struct nyas_job *next = nyas_arr_push(s->queue);
	*next = job;
	pthread_cond_signal(&s->cond);
	pthread_mutex_unlock(&s->mtx);
}

void
nyas_sched_wait(nysched *s)
{
	if (!nyas_arr_count(s->threads)) {
		return;
	}

	while (nyas_arr_count(s->queue) || s->waiting != nyas_arr_count(s->threads)) {
		pthread_cond_broadcast(&s->cond);
		nanosleep((const struct timespec[]){ { 0, 50000000L } }, NULL);
	}
}

int
nyas_sched_destroy(nysched *s)
{
	pthread_mutex_lock(&s->mtx);
	s->state = SCHED_CLOSING;
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mtx);
	for (int i = 0; i < nyas_arr_count(s->threads); ++i) {
		pthread_join(s->threads[i], NULL);
	}
	nyas_arr_release(s->threads);
	nyas_arr_release(s->queue);
	pthread_mutex_lock(&s->mtx);
	pthread_mutex_unlock(&s->mtx);
	pthread_mutex_destroy(&s->mtx);
	pthread_cond_destroy(&s->cond);
	s->state = SCHED_CLOSED;

	pthread_mutex_lock(&scheds_mtx);
	if (s == scheds) {
		scheds = scheds->next;
	} else {
		nysched *i = scheds;
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
			return NYAS_ERR_INVALID_PTR;
		}
	}

	pthread_mutex_unlock(&scheds_mtx);
	NYAS_FREE(s);
	return NYAS_OK;
}
