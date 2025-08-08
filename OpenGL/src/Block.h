#pragma once

#include "world/UniBlockData.h"

#include <algorithm>
#include <unordered_map>

class Block
{
private:
	BlockLocalPosition m_LocalPosition;
	std::vector<unsigned int> m_UniqevertId;
public:
	BlockData m_BlockData;
	int m_VertexOffset = 0;
public:
	void AppendFace(Face face);
	void BuldMesh();

	void SetInChunkPosition(const BlockLocalPosition& pos);
	void Clear();
private:
	void AppendIndices(const unsigned int indicesSet[]);
};