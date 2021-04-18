#include <vector>
#include <utility>
#include <omp.h>
#include <cmath>
#include <limits.h>
#include <algorithm>

class RRTNode 
{
public:
    RRTNode(int idx, int p, std::vector<int> c, std::pair<double, double> pos);

    int getId();

    int getParent();
    void setParent(int p);

    std::vector<int> getChildren();
    void addChild(int child_idx);

    std::pair<double, double> getPosition();
    
private:
    int id; //index of RRTNode in tree vector
    int parent; //index of parent in tree vector;
    std::vector<int> children; //indices of children in tree vector;
    std::pair<double, double> position;
};

class RRTTree
{
public:
    RRTTree(std::pair<double, double> start_pos); //FIXME create root somewhere
    ~RRTTree();

    RRTNode nearestNeighbor(std::pair<double, double> pos, int start_idx, int end_idx);

    void addNode(int p, std::pair<double, double> pos);

    int size();

private:
    std::vector<RRTNode> nodes;
    Lock treeLock;
};