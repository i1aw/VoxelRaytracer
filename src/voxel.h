#pragma once
#include <vector>
#include "util.h"
#include "constants.h"

// octree node
struct OctreeNode {
	// either the count of non-zero children or the value of the voxel
	unsigned int value = 0;
	OctreeNode* parent = nullptr;
	OctreeNode** children = nullptr;
};

class Octree {
private:
	int height = 0;
	OctreeNode* root = nullptr;

	void initR_(OctreeNode*& cur, int curHeight);
	void destructorR_(OctreeNode*& cur);

public:
	Octree(int height);
	~Octree();
	
	OctreeNode* getNode(Vector3f pos);
	bool setNode(Vector3f pos, unsigned int value);

	int getSize();
	bool inWorld(Vector3f pos);

	unsigned int get(Vector3f pos);
	bool set(Vector3f pos, unsigned int value);

};