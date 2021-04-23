#include <vector>
#include <utility>
#include <omp.h>
#include <cmath>
#include <limits.h>
#include <pthread.h>
#include <algorithm>


// ========== RRTNode class ========== //

class RRTNode {
public:
    RRTNode(int idx, int p, std::vector<int> c, std::pair<int, int> pos);
    RRTNode(const RRTNode& rhs);
    RRTNode& operator=(const RRTNode& rhs);
    int getParent();
    void setParent(int p);
    std::vector<int> getChildren();
    void addChild(int child_idx);
    std::pair<int, int> getPosition();
    int getIdx();
    void setIdx(int idx);

    int idx; 			          //index of RRTNode in tree vector
    int parent; 		          //index of parent in tree vector;
    std::vector<int> children;    //indices of children in tree vector;
    std::pair<int, int> position;
};



// ========== RRTTree class ========== //

class RRTTree {
public:
    RRTTree();
    ~RRTTree();
    void createRoot(std::pair<int, int> start_pos);
    int nearest_neighbor_search_new(std::pair<int, int> q_rand, int t);
    void addNode(RRTNode new_node);
    RRTNode get_node(int idx);
    int get_size();

private:
    std::vector<RRTNode> nodes;
    pthread_mutex_t tree_lock;
};


// useful for the reduction phase
struct args_info {
    int chunk_sz;
    std::pair<int, int> q_rand;
    std::pair<int, double>* results;
    std::vector<RRTNode> tree_nodes;
};
