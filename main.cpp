#include <random>
#include <pthread.h>
#include <stdio.h>

#include "tree.h"
#include "bmp2Map.h"
#include "barrier.hpp"

using namespace std;


// Struct for passing arguments into the function of pthread_create()
struct sample_args {
    int t;
    int m;
    int delta;
    int samples;
    int** pointer_to_bmpMap;
};

// RRTTree: Global structure for storing nodes
RRTTree tree = RRTTree();

pair<double, double> uniformSample(default_random_engine &generator, uniform_real_distribution &distribution){
    double x_pos = distribution(generator);
    double y_pos = distribution(generator);
    pair<double, double> p = make_pair(x_pos, y_pos);
    return p;
}

void some_sample_func(void *args) {
    // cast the void to a sample_args pointer
    struct sample_args *arguments = (sample_args*) args;
    
}

int main(int argc, char* argv){
    /*
    ===================================
    -----------------------
    Command Line Arguments:
    -----------------------
    • argv[1]: bitmap filename
    • argv[2]: x coordinate
    • argv[3]: y coordinate
    • argv[4]: # of sampler threads
    • argv[5]: # of samples
    • argv[6]: local sampling batch
    • argv[7]: delta
    • TODO... greediness parameters
    ===================================
    */
   
    // parse arguments
    char* bmp_filename = argv[1];
    int x = atoi(argv[2]);
    int y = atoi(argv[3]);
    int t = atoi(argv[4]);
    int samples = atoi(argv[5]);
    int m = atoi(argv[6]);
    int delta = atoi(argv[7]);

    // =================== Bmp2Map stuff can go here? =====================
    // * 
    // *
    // ====================================================================

    // TODO find x_max and y_max (width and height of bmp)
    double x_max = 1.0;
    double y_max = 3.7;

    // initialize the start state with the RRT Tree
    pair<int, int> start_state;
    start_state.first = x;
    start_state.second = y;
    tree.createRoot(start_state);
    
    // initialize random sampler
    // NOTE: to get random point: pair<double, double> result = uniformSample(generator, distribution);
    default_random_engine generator;
    uniform_real_distribution<double> distribution(x_max, y_max);

    // ======== Begin the Sampling Process ======== //

    // format arguments
    struct sample_args sample_func_args{t, m, delta, samples, bmpMapPointer};

    pthread_t thread_ids[t];
    for (int i = 0; i < t; ++i) {
        pthread_create(&thread_ids[i], NULL, &some_sample_func, (void*) &sample_args);
    }
    
    // wait for threads to finish
    for (int i = 0; i < t; ++i) {
        pthread_join(i, NULL);
    }

    return 0;
}

            