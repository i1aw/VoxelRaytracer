#pragma once
#include <vector>
#include "util.h"
#include "constants.h"

// octree node
struct Node {
	// either the count of non-zero children or the value of the voxel
	unsigned int value = 0;
	Node* parent = nullptr;
	Node** children = nullptr;
};

class Octree {
private:
	int height = 0;
	Node* root = nullptr;

	void initR_(Node*& cur, int curHeight);
	void destructorR_(Node*& cur);

public:
	Octree(int height);
	~Octree();
	
	Node* getNode(Vector3f pos);
	int getSize();
	bool inWorld(Vector3f pos);

	unsigned int get(Vector3f pos);
	bool set(Vector3f pos, unsigned int value);

};