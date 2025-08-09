#pragma once
#include <vector>

inline constexpr unsigned int ChunkSizeXY = 16;
inline constexpr unsigned int ChunkHeight = ChunkSizeXY * 4;

enum class ChunkState
{
	Undefined,
	TerrainGenerated,
	NeighboursGenerated,
	MeshGenerated,
	ChunkAllocated,
};

struct VertexBufferLayout
{
	int offset = -1;
	int size = -1;
};

struct IndexBufferLayout
{
	int offset = -1;
	int size = -1;
};

struct ChunkMesh
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	VertexBufferLayout vboLayout;
	IndexBufferLayout iboLayout;
};