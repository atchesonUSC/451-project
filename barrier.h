#ifndef barrier_h
#define barrier_h

#include <stdio.h>
#include <pthread.h>
#include <limits.h>


class Barrier {
    private:
        int nthreads;
        int count;
        pthread_mutex_t lock;
        pthread_cond_t all_here;
    public:
        Barrier();
        ~Barrier();
        void wait(void);
        void set_nthreads(int n);
};

#endif /* barrier_h */
