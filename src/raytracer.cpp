#include "raytracer.h"

RayTracer::RayTracer(int width, int height) {
	this->width = width;
	this->height = height;

	textureData = new unsigned char[width * height * 4];

    fov = FOV * (PI_F / 180.0f);
    fd = (width / 2) / tan(fov / 2);
    fdSq = fd * fd;
    updateVectors();

}

RayTracer::~RayTracer() {
	if (textureData) {
		delete[] textureData;
	}
}

void RayTracer::set(unsigned int x, unsigned int y, RGBColor color) {
	unsigned int index = (y * width + x) * 4;
	textureData[index + 0] = color.red;
	textureData[index + 1] = color.green;
	textureData[index + 2] = color.blue;
	textureData[index + 3] = 255u;

}

void RayTracer::updateVectors() {
    forward.x = sin(yaw) * cos(pitch);
    forward.y = sin(pitch);
    forward.z = cos(yaw) * cos(pitch);

    up.x = sin(pitch) * sin(yaw);
    up.y = -cos(pitch);
    up.z = sin(pitch) * cos(yaw);

    right.x = cos(yaw);
    right.y = 0;
    right.z = -sin(yaw);
}

void RayTracer::moveForward(float amount) {
    position += forward * amount;
}

void RayTracer::moveUp(float amount) {
    position += up * amount;
}

void RayTracer::moveRight(float amount) {
    position += right * amount;

}

unsigned char* RayTracer::render(SparceVoxelOctree& world, int threadCount, int threadIndex) {

    int linesPerThread = height / threadCount;

    int startY = threadIndex * linesPerThread;

    int endY = (threadIndex + 1) * linesPerThread;
    if (threadCount == threadIndex + 1) {
        endY = height;
    }

    const int halfWidth = width / 2;
    const int halfHeight = height / 2;

    const RGBColor fill = { 255,0,0 };
    const RGBColor background = { 0,0,0 };

    for (int y = startY; y < endY; y++) {

        for (int x = 0; x < width; x++) {

            // cache this eventually
            float dist = sqrt(x * x + y * y + fdSq);

            Vector3f dir = make_vec3f((x - halfWidth), (y - halfHeight), fd);
            dir /= dist;

            Vector3f RotatedDir = dir.x * right;
            RotatedDir += dir.y * up;
            RotatedDir += dir.z * forward;
            
            float distance = 0;
            const RGBColor* color = cast(world, position, RotatedDir, distance);

            if (color == nullptr) {

                set(x, y, background);

            }
            else {
                set(x, y, *color);

            }
        }
    }

    return textureData;
}

bool WorldRayIntersction(SparceVoxelOctree& world, Vector3f& pos, Vector3f dir) {
    // make 'slabs' for each axis, with an entry and exit 'time' (distance traveled)
    // find the maximum entry time and the minimum exit time
    // if the entry time is before the exit time, we have an intersection, otherwise we do not
    // set position to the location of the ray when it enters

    // if we have already intersected the box, return true
    if (world.inWorld(pos)) {
        return true;
    }

    int worldSize = world.getSize();

    // check if we are not moving in a specific axis.
    // if we are not, we have to already be inside of the slab to intersect
    if (fabs(dir.x) <= 1e-8) {
        if (pos.x < 0 || pos.x > worldSize) {
            return false;
        }
    }
    if (fabs(dir.y) <= 1e-8) {
        if (pos.y < 0 || pos.y > worldSize) {
            return false;
        }
    }
    if (fabs(dir.z) <= 1e-8) {
        if (pos.z < 0 || pos.z > worldSize) {
            return false;
        }
    }


    // find intersection time from start side (0) (may not be entry time)
    Vector3f entryTime;

    entryTime.x = dir.x != 0 ? -pos.x / dir.x : INFINITY;
    entryTime.y = dir.y != 0 ? -pos.y / dir.y : INFINITY;
    entryTime.z = dir.z != 0 ? -pos.z / dir.z : INFINITY;

    Vector3f exitTime;

    exitTime.x = dir.x != 0 ? (worldSize - pos.x) / dir.x : INFINITY;
    exitTime.y = dir.y != 0 ? (worldSize - pos.y) / dir.y : INFINITY;
    exitTime.z = dir.z != 0 ? (worldSize - pos.z) / dir.z : INFINITY;

    if (exitTime.x < entryTime.x) swap(exitTime.x, entryTime.x);
    if (exitTime.y < entryTime.y) swap(exitTime.y, entryTime.y);
    if (exitTime.z < entryTime.z) swap(exitTime.z, entryTime.z);

    float maxEntryTime = maxFinite(entryTime.x, entryTime.y, entryTime.z);
    float minExitTime = fmin(exitTime.x, fmin(exitTime.y, exitTime.z));

    if (minExitTime < 0) {
        return false;
    }
    if (maxEntryTime < minExitTime) {
        pos += (maxEntryTime + 1e-6) * dir;
        return true;
    }
    else {
        return false;
    }

}

const RGBColor* RayTracer::cast(SparceVoxelOctree& world, const Vector3f& pos, const Vector3f& dir, float& dist) {

    Vector3f curentPos = pos;

    // we never hit the world :(
    // this is also causing lag problems sometimes, maybe work is not evenly distrubuted among the threads
    if (!WorldRayIntersction(world, curentPos, dir)) {
        dist = INFINITY;
        return nullptr;
    }

    static const int MAX_T = 100;

    Vector3f currentVoxel = make_vec3f(floor(curentPos.x), floor(curentPos.y), floor(curentPos.z));

    if (world.get(currentVoxel)) {
        dist = 0;
        return world.get(currentVoxel);
    }

    // the direction that we step in which is +/- 1
    Vector3f step = make_vec3f(
        dir.x < 0 ? -1 : 1,
        dir.y < 0 ? -1 : 1,
        dir.z < 0 ? -1 : 1
    );

    // distance from the origin to the next boundary
    Vector3f tMax = make_vec3f(
        dir.x != 0 ? ((dir.x > 0 ? 1 : 0) - curentPos.x + currentVoxel.x) / dir.x : INFINITY,
        dir.y != 0 ? ((dir.y > 0 ? 1 : 0) - curentPos.y + currentVoxel.y) / dir.y : INFINITY,
        dir.z != 0 ? ((dir.z > 0 ? 1 : 0) - curentPos.z + currentVoxel.z) / dir.z : INFINITY
    );

    // distance 't' for ray to move right one voxel.
    Vector3f tDelta = make_vec3f(
        dir.x != 0 ? abs(1 / dir.x) : INFINITY,
        dir.y != 0 ? abs(1 / dir.y) : INFINITY,
        dir.z != 0 ? abs(1 / dir.z) : INFINITY
    );

    for (int t = 0; t < MAX_T; t++) {
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            dist = tMax.x;
            tMax.x += tDelta.x;
            currentVoxel.x += step.x;
        }
        else if (tMax.y < tMax.z) {
            dist = tMax.y;
            tMax.y += tDelta.y;
            currentVoxel.y += step.y;
        }
        else {
            dist = tMax.z;
            tMax.z += tDelta.z;
            currentVoxel.z += step.z;
        }

        const RGBColor* color = world.get(currentVoxel);
        if (color) {
            return color;

        }
    }

    dist = INFINITY;
    return nullptr;
}