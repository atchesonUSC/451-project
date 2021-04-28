#include "tree.h"
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>


// ========== RRTNode ========== //

RRTNode::RRTNode(int idx, int p, std::vector<int> c, std::pair<int, int> pos) : idx(idx), parent(p), children(c), position(pos) {}

RRTNode::RRTNode(const RRTNode& rhs)
:idx(rhs.idx)
, parent(rhs.parent)
, children(rhs.children)
, position(rhs.position) {

}

RRTNode& RRTNode::operator=(const RRTNode& rhs) {
	// 1. First check that we're not self-assigning
    if (&rhs != this) {
        this->idx = rhs.idx;
		int parent = rhs.parent;
		this->parent = parent;

		int pos1 = rhs.position.first;
		int pos2 = rhs.position.second;
		this->position = std::make_pair(pos1, pos2);
        
		this->children.clear();
        for (std::size_t i = 0; i < rhs.children.size(); ++i) {
            this->children.push_back(rhs.children[i]);
        }
    }

    return *this;
}

int RRTNode::getIdx(){
    return idx;
}

int RRTNode::getParent(){
    return parent;
}

std::vector<int> RRTNode::getChildren(){
    return children;
}

void RRTNode::setParent(int p){
    parent = p;
}

void RRTNode::addChild(int c){
    children.push_back(c);
}

std::pair<int, int> RRTNode::getPosition(){
    return position;
}

void RRTNode::setIdx(int idx) {
	this->idx = idx;
}

// ========== RRTTree ========== //

RRTTree::RRTTree() {
	// initialize the goal node index
	int goal_node_idx = -1;
}

RRTTree::~RRTTree() {}

void RRTTree::createRoot(std::pair<int, int> start_pos) {
	std::vector<int> empty;
	RRTNode root = RRTNode(0, -1, empty, start_pos);
	nodes.push_back(root);
}

void RRTTree::addNode(RRTNode new_node, int end_flag){
	int parent_idx, new_node_idx;

	parent_idx = new_node.getParent();

	new_node_idx = this->nodes.size();
	new_node.setIdx(new_node_idx);

	// check if this is the terminating node
	if (end_flag) {
		goal_node_idx = new_node_idx;
	}

        nodes[parent_idx].addChild(new_node_idx);
        nodes.push_back(new_node);
}

RRTNode RRTTree::get_node(int idx) {
	return nodes[idx];
}

int RRTTree::get_size() {
	return nodes.size();
}

void RRTTree::print_tree() {
    for (int i = 0; i < this->get_size(); ++i) {
        std::pair<int, int> pos = nodes[i].getPosition();
        int x = pos.first;
        int y = pos.second;
    
        printf("(%d, %d)\n", x, y);
    }
}

void RRTTree::serializeTree(std::ostream &o){
	o << nodes.size() << "\n";
	for(int i = 0; i < nodes.size(); i++){
		o << nodes[i].position.first << " " << nodes[i].position.second << " ";
		if(i >= 0){
			o << nodes[nodes[i].parent].position.first << " " << nodes[nodes[i].parent].position.second;
		}
		o << "\n";
	}
	int curr = goal_node_idx;
	std::vector<int> path;

	do{
		path.push_back(curr);
		curr = nodes[curr].parent;
	} while(curr != 0);

	o << path.size() << "\n";
	for(int i = 0; i < path.size(); i++){
		o << nodes[path[i]].position.first << " " << nodes[path[i]].position.second << "\n";
	}

}
