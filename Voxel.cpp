#include "Voxel.h"

unsigned int& VoxelSpace::at(int w, int h, int d) {

	return voxels[w * width * height + h * height + d];
}

unsigned int VoxelSpace::get(int w, int h, int d) {

	if (w >= width || w < 0 || h >= height || h < 0 || d >= depth || d < 0) {
		return 0;
	}

	return voxels[w * width * height + h * height + d];
}

unsigned int& VoxelSpace::at(Vector3f pos) {
	return at(floor(pos.x), floor(pos.y), floor(pos.z));
}
unsigned int VoxelSpace::get(Vector3f pos) {
	return get(floor(pos.x), floor(pos.y), floor(pos.z));
}

// Octree logic

Octree::Octree(int height) {
	this->height = height;

	initR_(root, 0);
}

Octree::~Octree() {
	destructorR_(root);
}

void Octree::destructorR_(Node*& cur) {
	if (!cur) return;

	if (cur->children) {
		for (int i = 0; i < 8; i++) {
			destructorR_(cur->children[i]);
		}
	}

	delete cur;
	cur = nullptr;
}

/*void Octree::initR_(Node*& cur, int depth) {
	if (depth + 1 == height) {
		cur = new Node();
		return;
	}

	// create new node
	cur = new Node;
	cur->value = 0;
	cur->children = new Node*[8];

	// create children of node
	for (int i = 0; i < 8; i++) {
		cur->children[i] = nullptr;
		initR_(cur->children[i], depth + 1);
		cur->children[i]->parent = cur;
	}
}*/

void Octree::initR_(Node*& cur, int depth) {
	cur = new Node;
	if (depth == height) {
		// quit if this is a leaf node, no children
		return;
	}
	else {

		// node has children
		cur->children = new Node*[8];

		for (int i = 0; i < 8; i++) {
			cur->children[i] = nullptr;
			initR_(cur->children[i], depth + 1);
			cur->children[i]->parent = cur;
		}
	}
}

Node* Octree::getNode(Vector3f pos) {
	static Node* null = nullptr;
	Vector3f curPos = pos;

	const int sideLength = 1 << height;
	if (pos.x > sideLength - 1 || pos.x < 0) return null;
	if (pos.y > sideLength - 1 || pos.y < 0) return null;
	if (pos.z > sideLength - 1 || pos.z < 0) return null;

	int curSide = sideLength;
	Node* cur = root;

	// node 0 is the left, bottom, near node (in that order)
	// moves right first, then up, then out

	while (cur->children) { // not a leaf node
		unsigned int nextIndex = 0;
		curSide >>= 1;

		if (curPos.x >= curSide) {
			nextIndex |= 1 << 0;
			curPos.x -= curSide;
		}
		if (curPos.y >= curSide) {
			nextIndex |= 1 << 1;
			curPos.y -= curSide;
		}
		if (curPos.z >= curSide) {
			nextIndex |= 1 << 2;
			curPos.z -= curSide;
		}

		cur = cur->children[nextIndex];
	}

	return cur;
}

unsigned int Octree::get(Vector3f pos) {
	Node* node = getNode(pos);

	if (node != nullptr) {
		return node->value;
	}
	else {
		return 0;
	}
}

/*
bool Octree::set(Vector3f pos, unsigned int value) {
	Node* n = getNode(pos);
	int change = value == 0 ? -1 : 1;

	if (n == nullptr) return false;

	// redundant call
	if (n->value == value) return true;

	if (n->value != 0 && change == 1) {
		// setting to another non-zero value, no need to update parents
		n->value = value;
		return true;
	}
	
	n->value = value;

	while (n->parent) {
		n = n->parent;
		n->value += change;
		
		// check if this changed the count of parent
		// if value is 1, add one to parents
		// if value is 0, subtract one from parent
		if (n->value > 1) break; // already had value, no change in parent
		
	}
	return true;
}
*/

bool Octree::set(Vector3f pos, unsigned int value) {
	Node* n = getNode(pos);
	if (n == nullptr) return false; // check if node is valid
	if (n->value == value) return true; // no change needed

	// changing a voxel that exists
	unsigned int old = n->value;
	int change = 0;
	n->value = value;

	if (old != 0 && value == 0) {
		// removing voxel, change is negative
		change = -1;
	}
	else if (old == 0) {
		// turning on voxel, change is positive
		change = 1;
	}
	else {
		// switching the state of voxel, no change is needed.
		return true;
	}

	// now we need to update the parents
	while (n->parent) {
		n = n->parent;
		old = n->value;
		n->value += change;

		// parent was already counting this child as on, no more changes needed
		if (change == 1 && old != 0) {
			break;
		}
	}
	
	return true;

}

int Octree::getSize() {
	return 1 << height;
}

bool Octree::inWorld(Vector3f pos) {
	int worldSize = getSize();

	return pos.x >= 0 && pos.x <= worldSize &&
		   pos.y >= 0 && pos.y <= worldSize &&
		   pos.z >= 0 && pos.z <= worldSize;
}