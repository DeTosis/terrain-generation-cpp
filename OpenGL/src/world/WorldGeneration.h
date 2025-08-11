#pragma once

#include <thread>
#include <future>
#include <unordered_map>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include "glm/glm.hpp"

#include "UniChunkData.h"
#include "WorldChunk.h"
#include "../Camera.h"

struct ChunkWithDist
{
	std::pair<int, int> coord;
	float dist;
};

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
	FastNoiseLite m_WorldNoise;
	std::unordered_map<std::pair<int,int>, WorldChunk*, PairHash> m_LoadedChunks;
	std::unordered_set<std::pair<int,int>, PairHash> m_ChunksInRenderDistance;
	std::vector<std::thread> m_WorkingThreads;
	bool m_StopWorkers;
	std::condition_variable m_GenCVar;
public:
	std::queue<WorldChunk*> m_GenerationQueue;
	std::vector<WorldChunk*> m_ChunksToRender;
public:
	WorldGeneration(int seed, int maxThreads);
	~WorldGeneration();
	void UpdateChunksInRenderDistance(int chunkRenderDistance, Camera& camera);
	void PrepareChunksForDraw(VertexBuffer& vb, IndexBuffer& ib);
public:
	void PreGenerateChunk(int worldX, int worldY);
	void PostGenerateChunks();
	void AllocateChunk(VertexBuffer& vb, IndexBuffer& ib,int worldX, int worldY);
	void DeallocateChunk(VertexBuffer& vb, IndexBuffer& ib, int worldX, int worldY);
private:
	void Enque(WorldChunk* chunk);
	void GenerationWorkerLoop();
	void UpdateNeighbours(WorldChunk* chunk);
	void GenerateNoise(int seed);
	WorldChunk* GetChunk(int worldX, int worldY);
};