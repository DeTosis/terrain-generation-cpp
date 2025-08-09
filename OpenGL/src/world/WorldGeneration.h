#pragma once
#include <algorithm>
#include <future>
#include "glm/glm.hpp"

#include "UniChunkData.h"
#include "WorldChunk.h"
#include "../Camera.h"

struct ChunkWithDist
{
	std::pair<int, int> coord;
	float dist;
};

struct ChunksMap
{
	std::pair<int, int> coords;
	WorldChunk* chunk;
};

class WorldGeneration
{
private:
	FastNoiseLite m_worldNoise;
	std::vector<ChunksMap> m_LoadedChunks;
	std::vector<std::pair<int,int>> m_ChunksInRenderDistance;
public:
	std::vector<WorldChunk*> m_ChunksToRender;
public:
	WorldGeneration(int seed);
	void UpdateChunksInRenderDistance(int chunkRenderDistance, Camera& camera);
	void GenerateVisibleChunks();
	void PrepareChunksForDraw(VertexBuffer& vb, IndexBuffer& ib);

public:
	void GenerateChunk(int worldX, int worldY);

	void AllocateChunk(VertexBuffer& vb, IndexBuffer& ib,int worldX, int worldY);
	void DeallocateChunk(VertexBuffer& vb, IndexBuffer& ib, int worldX, int worldY);
private:
	void GenerateNoise(int seed);
	bool IsChunkLoaded(int worldX, int worldY);
	WorldChunk* GetChunk(int worldX, int worldY);
};