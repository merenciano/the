#include "core/log.h"
#include "core/nyas_core.h"
#include "sched.h"
#include "utils/array.h"
#include <pthread.h>

static enum sched_states {
	SCHED_UNINIT = 0,
	SCHED_RUNNING,
	SCHED_CLOSING,
	SCHED_CLOSED
} state = SCHED_UNINIT;

static struct nyas_job *queue;
static pthread_t *threads;
static pthread_mutex_t mtx;
static pthread_cond_t cond;
static int waiting = 0;

static void *
nyas__worker(void *data)
{
	(void)data;
	while (1) {
		++waiting;
		pthread_mutex_lock(&mtx);
		while (!nyas_arr_count(queue)) {
			pthread_cond_wait(&cond, &mtx);
			if (state == SCHED_CLOSING) {
				--waiting;
				pthread_mutex_unlock(&mtx);
				goto exit_worker;
			}
		}

		nyas_arr_pop(queue);
		struct nyas_job *job = &queue[nyas_arr_count(queue)];
		--waiting;
		pthread_mutex_unlock(&mtx);
		(*(job->job))(job->args);
	}

exit_worker:
	return NULL;
}

int
nyas_sched_init(int thread_count, int queue_capacity)
{
	threads = nyas_arr_create(pthread_t, thread_count);
	queue = nyas_arr_create(struct nyas_job, queue_capacity);

	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&cond, NULL);

	nyas_arr_push(threads, thread_count);
	for (int i = 0; i < nyas_arr_count(threads); ++i) {
		if (pthread_create(threads + i, NULL, nyas__worker, NULL)) {
			NYAS_LOG_ERR("Thread creation error.");
			return NYAS_ERR_THREAD;
		}
	}

	state = SCHED_RUNNING;
	return NYAS_OK;
}

void
nyas_sched_do(struct nyas_job job)
{
	pthread_mutex_lock(&mtx);
	struct nyas_job *next = nyas_arr_push(queue);
	*next = job;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mtx);
}

void
nyas_sched_wait(void)
{
	pthread_mutex_lock(&mtx);
	while (waiting != nyas_arr_count(threads)) {
		nanosleep((const struct timespec[]){ { 0, 50000000L } }, NULL);
	}
	pthread_mutex_unlock(&mtx);
}

int
nyas_sched_destroy(void)
{
	pthread_mutex_lock(&mtx);
	state = SCHED_CLOSING;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mtx);
	for (int i = 0; i < nyas_arr_count(threads); ++i) {
		pthread_join(threads[i], NULL);
	}
	nyas_arr_release(threads);
	nyas_arr_release(queue);
	pthread_mutex_lock(&mtx);
	pthread_mutex_unlock(&mtx);
	pthread_mutex_destroy(&mtx);
	pthread_cond_destroy(&cond);
	state = SCHED_CLOSED;
	return NYAS_OK;
}
