#pragma once
#include "NoiseGen.h"

#include "UniChunkData.h"
#include "../Block.h"

#include "../VertexBuffer.h"
#include "../IndexBuffer.h"

class WorldChunk;
struct NeighbourChunks
{
	WorldChunk* xNeg = nullptr;
	WorldChunk* xPos = nullptr;
	WorldChunk* yNeg = nullptr;
	WorldChunk* yPos = nullptr;
};

class WorldChunk
{
private:
	FastNoiseLite* m_Noise = nullptr;
	NoiseGenerator m_NoiseGen;
	
	unsigned int m_VertexSize = 6;
	std::vector<BlockType> m_Terrain;
	NeighbourChunks m_Neighbours;
	ChunkMesh* m_Mesh = new ChunkMesh();
	ChunkState m_ChunkState = ChunkState::Undefined;
public:
	int m_WorldX = 0;
	int m_WorldY = 0;
public:
	WorldChunk(FastNoiseLite& noise);
	void SetWorldPosition(int worldX, int worldY);
	void SetNeighbours(NeighbourChunks neighbours);
	void GenerateTerrain();
	void GenerateMesh();
public:
	void AllocateChunk(VertexBuffer& vb, IndexBuffer& ib);
	void DeallocateChunk(VertexBuffer& vb, IndexBuffer& ib);

	bool IsEmptyNeighbours();

	bool GetTerrainAtIndex(BlockType& outType, int x, int y, int z);
	int GetIndex(int x, int y, int z) const;
	std::vector<BlockType>* GetTerrain();
	ChunkMesh* GetMesh() const;
	ChunkState GetState() const;
	void SetState(ChunkState state) ;
private:
	void CreateMesh();
};