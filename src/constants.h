#pragma once
#include <climits>

// math
const float PI_F = 3.14159265f;
static const float HALF_PI = 3.14159265f / 2.0;

//rendering
const float upscaling = 2;
const float FOV = 70;
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const float minLighting = 0.5;

// controls
const float SENSITIVITY = 1.5;
const float MOVEMENT_SPEED = 2.5; // units / second

// world
const unsigned int WORLD_SIZE = 4;

// voxel get API 
static const unsigned int OUT_OF_MAP = UINT_MAX;
static const unsigned int AIR = 0;

