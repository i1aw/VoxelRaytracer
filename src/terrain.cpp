#include "terrain.h"

TerrainGenerator::TerrainGenerator() {
}

TerrainGenerator::~TerrainGenerator() {
}

void TerrainGenerator::Generate(SparceVoxelOctree& world) {
	CreateTerrain(world);
}

void TerrainGenerator::CreateTerrain(SparceVoxelOctree& world) {
	int size = world.getSize();
	int** heightMap = new int* [size];
	srand(time(0));

	for (int i = 0; i < size; i++) {
		heightMap[i] = new int[size];
	}

	for (int x = 0; x < size; x++) {
		for (int z = 0; z < size; z++) {

			heightMap[x][z] = rand() % (size / 2);

		}
	}

	for (int x = 0; x < size; x++) {
		for (int z = 0; z < size; z++) {
			for (int y = 0; y < heightMap[x][z]; y++) {
				world.set({(float)x,(float)y,(float)z}, { 125, 181, 116 });
			}
		}
	}


	for (int i = 0; i < size; i++) {
		delete[] heightMap[i];
	}
	delete[] heightMap;
}