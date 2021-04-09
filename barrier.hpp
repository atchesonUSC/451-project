#ifndef barrier_hpp
#define barrier_hpp

#include <stdio.h>
#include <pthread.h>
#include <limits.h>

class Barrier {
private:
    int nthreads;
    int count;
    // int phase;
    pthread_mutex_t lock;
    pthread_cond_t all_here;
public:
    Barrier(int t);
    ~Barrier(void);
    // int arrived(void);
    // int done(int p);
    void wait(void);
};

#endif /* barrier_hpp */
