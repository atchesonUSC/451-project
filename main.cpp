#include <random>
#include <string>
#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "tree.h"
#include "bmp2Map.h"
#include "barrier.hpp"

using namespace std;


// function for parsing file with configuration options
string* parse_config_file(string filename);
pair<double, double> uniformSample(default_random_engine &generator, uniform_real_distribution &distribution);
void sample_func(void* args);

// Struct for passing arguments into the function of pthread_create()
struct sample_args {
    int t;
    int m;
    int delta;
    int samples;
    int** bmp_map;
    int dim_x;
    int dim_y;
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
• TODO... greediness parameters
===================================
*/

// RRTTree: Global tree structure for storing nodes
RRTTree tree = RRTTree();

int main(int argc, char* argv){

    // parse argument file
    string* config;
    string config_filename;

    config_filename = string(argv[1]);
    config = parse_config_file(config_filename);
   
    // setup arguments
    string bmp_filename;
    int x_root, y_root, t, samples, m, delta, x_end, y_end;

    bmp_filename = config[0];

    x_root = stoi(config[1]);
    y_root = stoi(config[2]);
    x_end = stoi(config[7]);
    y_end = stoi(config[8]);

    t = stoi(config[3]);
    m = stoi(config[5]);
    delta = stoi(config[6]);
    samples = stoi(config[4]);

    /*
    =================== Bmp2Map stuff can go here? =====================
    |                                                                  |
    ====================================================================
    */

    // TODO find x_max and y_max (width and height of bmp)
    double x_max = 1.0;
    double y_max = 3.7;

    // initialize the start state with the RRT Tree
    pair<int, int> root;
    root.first = x_root;
    root.second = y_root;
    tree.createRoot(root);
    
    /*
    initialize random sampler
    NOTE: to get random point: pair<double, double> result = uniformSample(generator, distribution);
    */
    default_random_engine generator;
    uniform_real_distribution<double> distribution(x_max, y_max);

    // format arguments
    sample_args sample_func_args {t, m, delta, samples, bmpMapPointer};

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
    pair<double, double> p = make_pair(x_pos, y_pos);
    return p;
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
performs sampling operations
*/
void sample_func(void* args_struct) {
    // TODO: add argument for reference to bmpMap
    sample_args* args;
    int** global_map, local_map;
    int samples, t, m, delta, dim_x, dim_y;

    // setup arguments
    args = (sample_args*) args_struct;

    t = args->t;
    m = args->m;
    delta = args->delta;
    sampels = args->samples;
    global_map = args->bmp_map;
    dim_x = args->dim_x;
    dim_y = args->dim_y;
    
    // create local copy of bmp_map
    local_map = new int[dim_x][dim_y];

    for (int i = 0; i < dim_x; i++) {
        for (int j = 0; j < dim_y; ++j) {
            local_map[i][j] = global_map[i][j];
        }
    }

    // begin sampling
    int i = 0;
    while (i < samples / t) {
        // container for storing locally-sampled nodes
        vector<RRTNode> local_bin;

        for (int j = 0; j < m; ++j) {
            pair<double, double> q_rand;
            q_rand = uniformSample(...); // TODO: check in about the uniformSample function

            // TODO: complete rest of sampling code
        }
    }
}