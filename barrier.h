#include <stdio.h>
#include <pthread.h>
#include <limits.h>


class Barrier {
    private:
        int count;
        int nthreads;
        pthread_mutex_t barrier_lock;
        pthread_cond_t all_here;
    public:
        Barrier();
        ~Barrier();
        void wait(void);
        void set_nthreads(int n);
};
