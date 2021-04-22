#include <cmath>
#include <random>
#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <pthread.h>

#include "tree.h"
#include "bmp2Map.h"
#include "barrier.h"

using namespace std;


// function prototypes
void sample_func(void* args);
string* parse_config_file(string filename);
pair<int, int> sample_bmp_map(int dim_x, int dim_y);
RRTNode create_node(int q_near_idx, pair<int, int> q_rand, int delta);
pair<double, double> uniformSample(default_random_engine &generator, uniform_real_distribution &distribution);

// Struct for passing arguments into the function of pthread_create()
struct sample_args {
    int t;
    int m;
    int delta;
    int samples;
    int openmp_t;
    bmpMap* bmp_map;
    unsigned int dim_x;
    unsigned int dim_y;
};

/*
===================================
-----------------------
Command Line Arguments:
-----------------------
• argv[1]: bitmap filename
• argv[2]: x coordinate (root)
• argv[3]: y coordinate (root)
• argv[4]: # of sampler threads
• argv[5]: # of samples
• argv[6]: local sampling batch
• argv[7]: delta
• argv[8]: x coordinate (end)
• argv[9]: y coordinate (end)
• argv[10]: # of openMP threads
• TODO... greediness parameters
===================================
*/




// RRTTree: Global tree structure for storing nodes
RRTTree tree;

// for barrier sync
Barrier barrier;

int main(int argc, char* argv){

    // just for making sure we can compile
    exit(0)

    if (argc != 2) {
        cout << "Incorrect number of arguments: [program_name] [config_filename]" << endl;
        return -1;
    }

    // parse argument file
    string* config;
    string config_filename;

    config_filename = string(argv[1]);
    config = parse_config_file(config_filename);
   
    // setup arguments
    string bmp_filename;
    int x_root, y_root, t, samples, m, delta, x_end, y_end, openmp_t;

    bmp_filename = config[0];

    x_root = stoi(config[1]);
    y_root = stoi(config[2]);
    x_end = stoi(config[7]);
    y_end = stoi(config[8]);

    t = stoi(config[3]);
    m = stoi(config[5]);
    delta = stoi(config[6]);
    samples = stoi(config[4]);
    openmp_t = stoi(config[10]);

    // setup barrier
    barrier.set_nthreads(t);

    // read in the map, find dimensions
    bmpMap bmp_map(bmp_filename);

    unsigned int dim_x = bmp_map.get_width();
    unsigned int dim_y = bmp_map.get_height();

    // initialize the start state with the RRT Tree
    pair<int, int> root (x_root, y_root);
    tree.createRoot(root);
    
    /*
    initialize random sampler
    NOTE: to get random point: pair<double, double> result = uniformSample(generator, distribution);
    */
    // not sure if we need this part?
    // default_random_engine generator;
    // uniform_real_distribution<double> distribution(x_max, y_max);

    // format arguments
    sample_args sample_func_args {t, m, delta, samples, openmp_t, &bmp_map, dim_x, dim_y};

    // generate threads to run sampling
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





// ============================================ Helper Functions ============================================ //


pair<double, double> uniformSample(default_random_engine &generator, uniform_real_distribution &distribution) {
    double x_pos = distribution(generator);
    double y_pos = distribution(generator);
    pair<double, double> p (x_pos, y_pos);
    return p;
}


pair<int, int> sample_bmp_map(int dim_x, int dim_y) {
    int x;
    int y;
    pair<int, int> point;

    x = rand();
    while (x > dim_x || x < 0) {
        x = rand();
    }

    y = rand();
    while (y > dim_y || y < 0) {
        y = rand();
    }

    point.first = x;
    point.second = y;

    return point;
}


/*
parse configuration file (contains setup arguments)
*/
string* parse_config_file(string filename) {
    int argc;
    string line;
    string* args;
    ifstream config_file;
      
    // open file
    config_file.open(filename);

    // get argument count
    getline(config_file, line);
    argc = stoi(line);

    // create array for storing arguments as strings
    args = new string[argc];

    // get arguments
    for (int i = 0; i < argc; ++i) {
        getline(config_file, line);
        args[i] = line;
    }

    // return array of arguments
    return args;
}


/*
create new node in direction of sampled node at a distance of delta away from the nearest neighbor
*/
RRTNode create_node(int q_near_idx, pair<int, int> q_rand, int delta) {
    int q_rand_x = q_rand.first;
    int q_rand_y = q_rand.second;
    
    RRTNode rrt_node = tree.get_node(q_near_idx);
    pair<int, int> rrt_node_pos = rrt_node.getPosition();
    int rrt_node_x = rrt_node_pos.first;
    int rrt_node_y = rrt_node_pos.second;

    int delta_x = q_rand_x - rrt_node_x;
    int delta_y = q_rand_y - rrt_node_y;

    double angle = atan2(delta_y, delta_x);

    int x_new = (int) (cos(angle) * delta);
    int y_new = (int) (sin(angle) * delta);

    // create new node
    pair<int, int> pos_new (x_new, y_new);

    int p = q_near_idx;
    int idx = -1;   // leave this unknown for now, update when finally added to the tree
    vector<RRTNode> children;

    RRTNode new_node(idx, p, children, pos_new);
    
    return new_node;
}


/*
performs sampling operations
*/
void sample_func(void* args_struct) {
    sample_args* args;
    int samples, t, m, delta, dim_x, dim_y, openmp_t;

    // setup arguments
    args = (sample_args*) args_struct;

    bmpMap local_map(*args->bmp_map);

    t = args->t;
    m = args->m;
    delta = args->delta;
    samples = args->samples;
    dim_x = args->dim_x;
    dim_y = args->dim_y;
    openmp_t = args->openmp_t;

    // begin sampling
    int i = 0;
    while (i < samples / t) {
        // container for storing locally-sampled nodes
        vector<RRTNode> local_bin;

        // sync threads
        barrier.wait();

        for (int j = 0; j < m; ++j) {
            // sample random location on map
            pair<double, double> q_rand;
            q_rand = sample_bmp_map(dim_x, dim_y);

            // find nearest neighbor to q_rand
            int neighbor_idx = tree.nearest_neighbor_search(q_rand, openmp_t);

            // create new node for tree
            RRTNode new_node = create_node(neighbor_idx, q_rand, delta);

            //Check if too similar   
                // find nearest neighbor to q_new, if the same nnearest neighbor it is not too similar
            int neighbor_idx2 = tree.nearest_neighbor_search(new_node.getPosition(), openmp_t);
            pair<int, int> q_new ((int) new_node.getPosition().first, (int) new_node.getPosition().first);
            //if valid and not too similar pushback
            if (local_map.checkFree(q_new) && neighbor_idx == neighbor_idx2) {
                local_bin.push_back(new_node);
            }
        }

        // sync threads
        barrier.wait();

        // TODO: Add to global bin
        for (int i = 0; i < m; ++i) {
            tree.addNode(local_bin[i]);
        }
    }
}
