#pragma once
#include "world/UniChunkData.h"

#include "Block.h"
#include <fastNoise/FastNoiseLite.h>

#include "VertexBuffer.h"
#include "IndexBuffer.h"

class Chunk;
struct NeighbourChunks
{
	Chunk* xNeg = nullptr;
	Chunk* xPos = nullptr;
	Chunk* yNeg = nullptr;
	Chunk* yPos = nullptr;
};

class Chunk
{
private:
	unsigned int m_VertexSize = 6;
	int m_WorldX = 0;
	int m_WorldY = 0;
	BlockType m_Terrain[ChunkSizeXY][ChunkHeight][ChunkSizeXY];
public:
	ChunkState state = ChunkState::Undefined;
	ChunkMesh m_Mesh;
	NeighbourChunks m_Neighbours;
public:
		Chunk();
		size_t GetTerrainIndex(int x, int y, int z);
		void GenerateTerrain(FastNoiseLite& noise, int x, int y);
		void GenerateMesh();
		void SetNeighbours(NeighbourChunks neighbours);
public:
		void AllocateChunk(VertexBuffer& vb, IndexBuffer& ib);
		void Deallocate(VertexBuffer& vb, IndexBuffer& ib);
private:
		void CreateMesh(FastNoiseLite& noise);
		void SetWorldPosition(int x, int y);
		void GenerateBlocks();
};
