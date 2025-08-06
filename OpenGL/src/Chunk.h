#pragma once

#include <vector>

#include "Block.h"
#include <fastNoise/FastNoiseLite.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"


struct ChunkMesh
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
};

struct VBOLayout
{
	int offset = -1;
	int size = -1;
};

struct IBOLayout
{
	int offset = -1;
	int size = -1;
};

enum class RenderState
{
	NONE,
	GENERATED,
	ALLOCATED,
};

const static unsigned int m_ChunkSize = 16;
const static unsigned int m_ChunkHeight = m_ChunkSize * 2;

class Chunk
{
private:
	unsigned int m_VertexSize = 6;
	BlockType m_Chunk[m_ChunkSize][m_ChunkHeight][m_ChunkSize];
	int m_WorldX = 0;
	int m_WorldY = 0;
public:
	RenderState state = RenderState::NONE;
	ChunkMesh* m_MeshData = new ChunkMesh();
	VBOLayout m_VBOLayout;
	IBOLayout m_IBOLayout;
public:
		Chunk();
		~Chunk();
		void GenerateChunk(FastNoiseLite& noise, int x, int y);
		void AllocateChunk(VertexBuffer& vb, IndexBuffer& ib);
		void UnLoad(VertexBuffer& vb, IndexBuffer& ib);
private:
		void GenerateTerrain(FastNoiseLite& noise);
		void SetWorldPosition(int x, int y);
		void GenerateBlocks();
};
