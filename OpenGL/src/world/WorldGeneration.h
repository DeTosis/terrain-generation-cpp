#pragma once

#include <unordered_map>
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

//struct ChunksMap
//{
//	std::pair<int, int> coords;
//	WorldChunk* chunk;
//};

struct PairHash
{
	std::size_t operator()(const std::pair<int, int>& pair) const noexcept
	{
		return (std::hash<int>()(pair.first) << 1) ^ std::hash<int>()(pair.second);
	}
};

class WorldGeneration
{
private:
	FastNoiseLite m_worldNoise;

	std::unordered_map<std::pair<int,int>, WorldChunk*, PairHash> m_LoadedChunks;
	std::vector<std::pair<int,int>> m_ChunksInRenderDistance;
public:
	std::vector<WorldChunk*> m_ChunksToRender;
public:
	WorldGeneration(int seed);
	void UpdateChunksInRenderDistance(int chunkRenderDistance, Camera& camera);
	void GenerateVisibleChunks();
	void PrepareChunksForDraw(VertexBuffer& vb, IndexBuffer& ib);

public:
	void PreGenerateChunk(int worldX, int worldY);
	void UpdateChunks();

	void AllocateChunk(VertexBuffer& vb, IndexBuffer& ib,int worldX, int worldY);
	void DeallocateChunk(VertexBuffer& vb, IndexBuffer& ib, int worldX, int worldY);
private:
	void UpdateNeighbours(WorldChunk* chunk);
	void GenerateNoise(int seed);
	WorldChunk* GetChunk(int worldX, int worldY);
};