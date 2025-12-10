#include "util.h"
#include "SVO.h"
#include "cstdlib"
#include "ctime"

class TerrainGenerator {
private:
	void CreateTerrain(SparceVoxelOctree& world);

public:
	TerrainGenerator();
	~TerrainGenerator();

	void Generate(SparceVoxelOctree& world);

};