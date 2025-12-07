#pragma once
#include "util.h"

struct SVO_Node {
	// if leaf, the color of that voxel
	// if non-leaf, the average color of children nodes
	RGBColor color;

	// the children of the node
	SVO_Node** children = nullptr;

	// the parent node of this node
	SVO_Node* parent = nullptr;

	// the number of children who are not air
	uint8_t filledChildCount = 0;
};

class SparceVoxelOctree {
private:
	using Node = SVO_Node;

	int height = 0;
	int worldScale = 0;
	Node* root = nullptr;

public:
	
	// constructor and destructor
	SparceVoxelOctree(int height);
	~SparceVoxelOctree();

	// util functions
	int getSize();
	bool inWorld(Vector3f pos);

	// mutators and accessors

	// gets a voxel at a world position. Gives a depth limit.
	const RGBColor* get(Vector3f pos, int maxDepth = INT_MAX);
	const RGBColor* get(Vector3f pos, int& voxelSize, int maxDepth);


	// gets a voxel at a location and returns the depth at that location
	//const RGBColor* getMaxDepth(Vector3f pos, int& depth);

	// sets a voxel at a depth
	bool set(Vector3f pos, RGBColor color, int maxDepth = INT_MAX);

	// removes voxel at depth;
	bool remove(Vector3f pos, int maxDepth = INT_MAX);


};