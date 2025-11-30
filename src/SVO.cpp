#include "SVO.h"

void initChildren(SVO_Node* cur) {
	cur->children = new SVO_Node*[8];
	for (int i = 0; i < 8; i++) {
		cur->children[i] = new SVO_Node();
		cur->children[i]->parent = cur;
	}
}

void destroyR_(SVO_Node*& cur) {
	if (!cur) return;
	if (cur->children) {
		for (int i = 0; i < 8; i++) {
			destroyR_(cur->children[i]);
		}

		delete[] cur->children;
		cur->children = nullptr;
	}

	delete cur;
	cur = nullptr;
	return;
}

SparceVoxelOctree::SparceVoxelOctree(int height) {
	root = new Node();
	this->height = height;
	this->worldScale = 1u << height;
}

SparceVoxelOctree::~SparceVoxelOctree() {
	destroyR_(root);
}

int SparceVoxelOctree::getSize() {
	return worldScale;
}

bool SparceVoxelOctree::inWorld(Vector3f pos) {
	int worldSize = getSize();

	return pos.x >= 0 && pos.x <= worldSize &&
		pos.y >= 0 && pos.y <= worldSize &&
		pos.z >= 0 && pos.z <= worldSize;
}

void updateColorAveragesR_(SVO_Node* cur) {
	if (!cur) return;
	if (!cur->parent) return;

	uint16_t avgRed = 0;
	uint16_t avgGreen = 0;
	uint16_t avgBlue = 0;
	int childCount = 0;

	for (int i = 0; i < 8; i++) {
		SVO_Node* n = cur->parent->children[i];
		if (n->filledChildCount < 4) continue;

		childCount++;
		avgRed += n->color.red;
		avgGreen += n->color.green;
		avgBlue += n->color.blue;
	}

	bool changedToClear = cur->parent->filledChildCount >= 4 && childCount < 4;
	cur->parent->filledChildCount = childCount;

	// if there are less then half filled children, it will be clear
	// otherwise, we change the color and update its parent
	if (childCount >= 4) {
		avgRed /= childCount;
		avgGreen /= childCount;
		avgBlue /= childCount;

		cur->parent->color.red = avgRed;
		cur->parent->color.green = avgGreen;
		cur->parent->color.blue = avgBlue;
		updateColorAveragesR_(cur->parent);
	} else if (changedToClear) { // this partition was changed to clear, update parents
		updateColorAveragesR_(cur->parent);
	}
	
}

const SVO_Color* SparceVoxelOctree::get(Vector3f pos, int maxDepth) {
	if (!inWorld(pos)) return nullptr;
	int curDepth = 0;
	int sideLength = getSize();
	Node* cur = root;

	uint32_t x = floor(pos.x);
	uint32_t y = floor(pos.y);
	uint32_t z = floor(pos.z);

	while (curDepth < height && curDepth < maxDepth) {
		sideLength >>= 1;
		// there are no children, return early
		if (!cur->children) {
			return nullptr;
		}
		int nextIndex = 0;

		// check if our x/y/z is greater then half of the curent partition side length
		if (x >= sideLength) {
			x -= sideLength;
			nextIndex |= 1u;
		}
		if (y >= sideLength) {
			y -= sideLength;
			nextIndex |= 2u;
		}
		if (z >= sideLength) {
			z -= sideLength;
			nextIndex |= 4u;
		}

		cur = cur->children[nextIndex];

		curDepth++;
	}
	if (cur->filledChildCount >= 4) {
		return &cur->color;

	}
	else {
		return nullptr;
	}
}

bool SparceVoxelOctree::set(Vector3f pos, SVO_Color color, int maxDepth) {
	if (!inWorld(pos)) return false;

	uint32_t x = floor(pos.x);
	uint32_t y = floor(pos.y);
	uint32_t z = floor(pos.z);

	int curSide = getSize();
	int depth = 0;
	Node* cur = root;

	// node 0 is the left, bottom, near node (in that order)
	// moves right first, then up, then out

	while (cur) {
		unsigned int nextIndex = 0;
		curSide >>= 1;

		// found the bottom, set voxel
		if (maxDepth == depth || height == depth) {
			cur->color = color;
			// if the voxel had children, remove them
			if (cur->children) {
				for (int i = 0; i < 8; i++) {
					destroyR_(cur->children[i]);
				}
			}

			// all nodes within partition are considered non-air
			cur->filledChildCount = 8;
			updateColorAveragesR_(cur);


			return true;
		}

		// check to see if our next node exists yet
		if (cur->children == nullptr) {
			// if not, create children
			initChildren(cur);
		}

		// find next node to go to

		if (x >= curSide) {
			x -= curSide;
			nextIndex |= 1u;
		}
		if (y >= curSide) {
			y -= curSide;
			nextIndex |= 2u;
		}
		if (z >= curSide) {
			z -= curSide;
			nextIndex |= 4u;
		}

		cur = cur->children[nextIndex];
		depth++;
	}

	return true;
}

bool SparceVoxelOctree::remove(Vector3f pos, int maxDepth) {
	if (!inWorld(pos)) return false;

	uint32_t x = floor(pos.x);
	uint32_t y = floor(pos.y);
	uint32_t z = floor(pos.z);

	int curSide = getSize();
	int depth = 0;
	Node* cur = root;

	// node 0 is the left, bottom, near node (in that order)
	// moves right first, then up, then out

	while (cur) {
		unsigned int nextIndex = 0;
		curSide >>= 1;

		// found the bottom, set voxel
		if (maxDepth == depth || height == depth) {
			// if the voxel had children, remove them
			if (cur->children) {
				for (int i = 0; i < 8; i++) {
					destroyR_(cur->children[i]);
				}
			}

			// all nodes within partition are considered air
			cur->filledChildCount = 0;
			updateColorAveragesR_(cur);


			return true;
		}

		// check to see if our next node exists yet
		if (cur->children == nullptr) {
			// if not, create children
			initChildren(cur);
		}

		// find next node to go to

		if (x >= curSide) {
			x -= curSide;
			nextIndex |= 1u;
		}
		if (y >= curSide) {
			y -= curSide;
			nextIndex |= 2u;
		}
		if (z >= curSide) {
			z -= curSide;
			nextIndex |= 4u;
		}

		cur = cur->children[nextIndex];
		depth++;
	}

	return true;
}