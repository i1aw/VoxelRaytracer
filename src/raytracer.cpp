#include "raytracer.h"

RayTracer::RayTracer(int width, int height) {
	this->width = width;
	this->height = height;

	textureData = new unsigned char[width * height * 4];

    fov = FOV * (PI_F / 180.0f);
    fd = (width / 2) / tan(fov / 2);
    fdSq = fd * fd;
    updateVectors();

    brightnessMap[0] = fmax(make_vec3f( 1, 0, 0) * lightDir * -1, 0);
    brightnessMap[1] = fmax(make_vec3f(-1, 0, 0) * lightDir * -1, 0);
    brightnessMap[2] = fmax(make_vec3f( 0, 1, 0) * lightDir * -1, 0);
    brightnessMap[3] = fmax(make_vec3f( 0,-1, 0) * lightDir * -1, 0);
    brightnessMap[4] = fmax(make_vec3f( 0, 0, 1) * lightDir * -1, 0);
    brightnessMap[5] = fmax(make_vec3f( 0, 0,-1) * lightDir * -1, 0);

    // convert from percent to brightness using minLighting
    float scale = (1 - minLighting) / 2.0f;
    for (int i = 0; i < 6; i++) {
        brightnessMap[i] = (brightnessMap[i] + 1) * scale;
        brightnessMap[i] += minLighting;
    }

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
    int normal = 0;

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
            const RGBColor* color = fastCast(world, position, RotatedDir, distance, normal);

            if (color == nullptr) {

                set(x, y, background);

            }
            else {
                set(x, y, (*color) * (float)brightnessMap[normal]);
                //set(x, y, *color);


            }
        }
    }

    return textureData;
}

bool WorldRayIntersection(SparceVoxelOctree& world, Vector3f& pos, Vector3f dir, int& normal) {
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
        if (maxEntryTime = entryTime.x) normal = 0 + (dir.x > 0);
        if (maxEntryTime = entryTime.y) normal = 2 + (dir.y > 0);
        if (maxEntryTime = entryTime.z) normal = 4 + (dir.z > 0);
        pos += (maxEntryTime + 1e-6) * dir;
        return true;
    }
    else {
        return false;
    }

}

const RGBColor* RayTracer::fastCast(
    SparceVoxelOctree& world,
    const Vector3f& pos,
    const Vector3f& dir,
    float& dist,
    int& normal) {

    const static RGBColor* null = nullptr;

    const int MAX_STEPS = 20;
    int curStep = 0;

    float t = 0;
    int voxelSize = 1;


    Vector3f curPos = pos;
    Vector3f curVox = make_vec3f(floor(pos.x), floor(pos.y), floor(pos.z));
    int curScale = 1; // the current partition size we are stepping through
    int minScale = 1; // sets the 'resolution' of the cast, can be increased when ray is far away

    // quit if ray never enters world, otherwise return position where it hits
    if (!WorldRayIntersection(world, curPos, dir, normal)) {
        dist = INFINITY;
        return null;
    }

    // makes the scope of the hitVoxel variable not the whole function
    {
        const RGBColor* hitVoxel = world.get(curPos, curScale, 999);
        if (hitVoxel) {
            dist = 0;
            return hitVoxel;
        }
    }

    do {
#ifdef TESTING
        std::cout << "(" << (int)curPos.x << ", " << (int)curPos.y << ", " << (int)curPos.z << ") " << curScale << std::endl;
#endif
        
        voxelSize = 1 << (curScale - 1);

        // the direction that we step in which is +/- 1
        Vector3f step = make_vec3f(
            dir.x < 0 ? -1 : 1,
            dir.y < 0 ? -1 : 1,
            dir.z < 0 ? -1 : 1
        );

        // find the max distance 't' travelable before the next voxel border.
       

        Vector3f tMax = make_vec3f(
            dir.x > 0 ? voxelSize - fmod(curPos.x, voxelSize) : fmod(curPos.x, voxelSize),
            dir.y > 0 ? voxelSize - fmod(curPos.y, voxelSize) : fmod(curPos.y, voxelSize),
            dir.z > 0 ? voxelSize - fmod(curPos.z, voxelSize) : fmod(curPos.z, voxelSize)
        );
        
        if (tMax.x == 0)
            tMax.x = voxelSize;
        if (tMax.y == 0)
            tMax.y = voxelSize;
        if (tMax.z == 0)
            tMax.z = voxelSize;

        tMax.x = dir.x != 0 ? tMax.x / dir.x * step.x : INFINITY;
        tMax.y = dir.y != 0 ? tMax.y / dir.y * step.y : INFINITY;
        tMax.z = dir.z != 0 ? tMax.z / dir.z * step.z : INFINITY;

        if (tMax.x <= tMax.y && tMax.x <= tMax.z) {
            normal = 0 + (step.x > 0);
            t = tMax.x;
        }
        else if (tMax.y <= tMax.z) {
            normal = 2 + (step.y > 0);
            t = tMax.y;
        }
        else {
            normal = 4 + (step.z > 0);
            t = tMax.z;
        }

        t += 1e-5;

        curPos.x += t * dir.x;
        curPos.y += t * dir.y;
        curPos.z += t * dir.z;

        if (!world.inWorld(curPos)) {
            dist = INFINITY;
            return null;
        }

        const RGBColor* hitVoxel = world.get(curPos, curScale, 999);
        if (hitVoxel) {
            return hitVoxel;
        }

        curStep++;
    } while (curStep < MAX_STEPS);

}