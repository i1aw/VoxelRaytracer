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
	int** temp = new int* [size];
	auto startTime = time(0);
	srand(startTime);

	for (int i = 0; i < size; i++) {
		heightMap[i] = new int[size];
		temp[i] = new int[size];
	}

	// setting random values
	for (int x = 0; x < size; x++) {
		for (int z = 0; z < size; z++) {

			heightMap[x][z] = rand() % (size);

		}
	}

	// smoothing
	for (int i = 0; i < 10; i++) {
		// repeat smoothing to get effective result
		for (int x = 0; x < size; x++) {
			for (int z = 0; z < size; z++) {
				// set my height to the average of my neighbors height and my own height
				int avg = 0;
				avg += heightMap[goodMod(x - 1, size)][goodMod(z - 1, size)];
				avg += heightMap[goodMod(x + 0, size)][goodMod(z - 1, size)];
				avg += heightMap[goodMod(x + 1, size)][goodMod(z - 1, size)];
				avg += heightMap[goodMod(x - 1, size)][goodMod(z + 0, size)];
				avg += heightMap[goodMod(x + 0, size)][goodMod(z + 0, size)] * 2;
				avg += heightMap[goodMod(x + 1, size)][goodMod(z + 0, size)];
				avg += heightMap[goodMod(x - 1, size)][goodMod(z + 1, size)];
				avg += heightMap[goodMod(x + 0, size)][goodMod(z + 1, size)];
				avg += heightMap[goodMod(x + 1, size)][goodMod(z + 1, size)];
				
				avg = floor((float)avg / 10.0f);
				temp[x][z] = avg;
			}

			
		}
		
		for (int x = 0; x < size; x++) {
			for (int z = 0; z < size; z++) {
				heightMap[x][z] = temp[x][z];
			}
		}
	}

	// filling with height map
	for (int x = 0; x < size; x++) {
		for (int z = 0; z < size; z++) {
			for (int y = 0; y < heightMap[x][z]; y++) {
				world.set({(float)x,(float)y,(float)z}, { 125, 181, 116 });
			}
		}
	}

	for (int i = 0; i < size; i++) {
		delete[] heightMap[i];
		temp[i];
	}
	delete[] heightMap;
	delete[] temp;
}