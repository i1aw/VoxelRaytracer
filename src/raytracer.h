#pragma once
#include "Voxel.h"
#include <map>
#include <tuple>
#include "util.h"
#include "constants.h"

struct RGBColor {
	unsigned char red = 0u;
	unsigned char green = 0u;
	unsigned char blue = 0u;
};

struct collisionData {
	unsigned int voxelType = 0;
	float distance = 0;
};

class RayTracer {
private:
	int width = 0;
	int height = 0;

	Vector3f forward;
	Vector3f up;
	Vector3f right;

public:
	RayTracer(int width, int height);
	~RayTracer();

	unsigned char* render(Octree& world, int threadCount, int threadIndex);

	void cast(Octree& world, const Vector3f& pos, const Vector3f& dir, unsigned int& type, float& dist);
	void set(unsigned int x, unsigned int y, RGBColor color);

	void updateVectors();

	void moveForward(float distance);
	void moveUp(float distance);
	void moveRight(float distance);

	Vector3f position = make_vec3f(0.5, 0.5, 0.5);

	float pitch = 0;
	float yaw = 0;
	float fov = 0;
	float fd = 1;
	float fdSq = 1;

	unsigned char* textureData = nullptr;
};

