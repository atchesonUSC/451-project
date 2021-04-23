#include "barrier.h"


Barrier::Barrier() {
	// setup threads and count
	count = 0;
    nthreads = 0;

	// initialize lock and condition
	pthread_mutex_init(&barrier_lock, NULL);
	pthread_cond_init(&all_here, NULL);
}

Barrier::~Barrier() {}

void Barrier::wait() {
	// acquire lock
	pthread_mutex_lock(&barrier_lock);

	// update count of arrived threads
    count++;
    if (count == nthreads) {
        count = 0;
        pthread_cond_broadcast(&all_here);
    } else {
        pthread_cond_wait(&all_here, &barrier_lock);
    }

    // release lock
    pthread_mutex_unlock(&barrier_lock);
}

void Barrier::set_nthreads(int n) {
    nthreads = n;
}
