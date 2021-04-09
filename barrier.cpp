#include "barrier.hpp"

Barrier::Barrier(int t) {
	// setup threads and count
	count = 0;
	nthreads = t;

	// initialize lock and condition
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&all_here, NULL);
}

void Barrier::wait() {
	// acquire lock
	pthread_mutex_lock(&lock);

	// update count of arrived threads
    thread_count++;
    if (thread_count == t) {
        thread_count = 0;
        pthread_cond_broadcast(&all_here);
    } else {
        pthread_cond_wait(&all_here, &lock);
    }

    // release lock
    pthread_mutex_unlock(&lock);
}
