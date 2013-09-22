/*
 * Declarations for the clients of a thread pool.
 */

#include <pthread.h>

/*
 * The thr_pool_t type is opaque to the client.
 * It is created by thr_pool_create() and must be passed
 * unmodified to the remainder of the interfaces.
 */
typedef struct sstack_thread_pool sstack_thread_pool_t;
/*
 * Create a thread pool.
 *    min_threads:    the minimum number of threads kept in the pool,
 *            always available to perform work requests.
 *    max_threads:    the maximum number of threads that can be
 *            in the pool, performing work requests.
 *    linger:        the number of seconds excess idle worker threads
 *            (greater than min_threads) linger before exiting.
 *    attr:        attributes of all worker threads (can be NULL);
 *            can be destroyed after calling thr_pool_create().
 * On error, thr_pool_create() returns NULL with errno set to the error code.
 */
sstack_thread_pool_t *sstack_thread_pool_create(uint32_t  min_threads,
		uint32_t max_threads, uint32_t linger, pthread_attr_t *attr);

/*
 * Enqueue a work request to the thread pool job queue.
 * If there are idle worker threads, awaken one to perform the job.
 * Else if the maximum number of workers has not been reached,
 * create a new worker thread to perform the job.
 * Else just return after adding the job to the queue;
 * an existing worker thread will perform the job when
 * it finishes the job it is currently performing.
 *
 * The job is performed as if a new detached thread were created for it:
 *    pthread_create(NULL, attr, void *(*func)(void *), void *arg);
 *
 * On error, thr_pool_queue() returns -1 with errno set to the error code.
 */
int sstack_thread_pool_queue(sstack_thread_pool_t *pool,
		void *(*func)(void *), void *arg);

/*
 * Wait for all queued jobs to complete.
 */
void sstack_thread_pool_wait(sstack_thread_pool_t *pool);

/*
 * Cancel all queued jobs and destroy the pool.
 */
void sstack_thread_pool_destroy(sstack_thread_pool_t *pool);
