#include "Block.h"

void Block::AppendFace(Face face)
{
	switch (face)
	{
	case Face::BACK:
		AppendIndices(IndicesBackFace);
		break;
	case Face::FRONT:
		AppendIndices(IndicesFrontFace);
		break;
	case Face::LEFT:
		AppendIndices(IndicesLeftFace);
		break;
	case Face::RIGHT:
		AppendIndices(IndicesRightFace);
		break;
	case Face::TOP:
		AppendIndices(IndicesTopFace);
		break;
	case Face::BOTTOM:
		AppendIndices(IndicesBottomFace);
		break;
	case Face::ALL:
		AppendFace(Face::BACK);
		AppendFace(Face::FRONT);
		AppendFace(Face::LEFT);
		AppendFace(Face::RIGHT);
		AppendFace(Face::TOP);
		AppendFace(Face::BOTTOM);
		break;
	}
}
void Block::BuldMesh()
{
	std::sort(m_UniqevertId.begin(), m_UniqevertId.end());
	m_UniqevertId.erase(std::unique(m_UniqevertId.begin(), m_UniqevertId.end()), m_UniqevertId.end());

	std::unordered_map<unsigned int, unsigned int> idRemap;

	for (int i = 0; i < m_UniqevertId.size(); i++)
	{
		unsigned int oldId = m_UniqevertId[i];
		idRemap[oldId] = i;

		// POSITIONS
		m_BlockData.blockVertices.push_back(CubeVerticesSet[oldId][0] + m_LocalPosition.x);
		m_BlockData.blockVertices.push_back(CubeVerticesSet[oldId][1] + m_LocalPosition.y);
		m_BlockData.blockVertices.push_back(CubeVerticesSet[oldId][2] + m_LocalPosition.z);
		// COLORS
		m_BlockData.blockVertices.push_back(CubeVerticesSet[oldId][3]);
		m_BlockData.blockVertices.push_back(CubeVerticesSet[oldId][4]);
		m_BlockData.blockVertices.push_back(CubeVerticesSet[oldId][5]);
	}

	for (int i = 0; i < m_BlockData.blockIndices.size(); i++)
	{
		m_BlockData.blockIndices[i] = idRemap[m_BlockData.blockIndices[i]] + m_VertexOffset;
	}
}

void Block::SetInChunkPosition(const BlockLocalPosition& pos)
{
	m_LocalPosition = pos;
}

void Block::AppendIndices(const unsigned int indicesSet[])
{
	for (int i = 0; i < 6; i++)
	{
		auto value = indicesSet[i];
		m_UniqevertId.push_back(value);
		m_BlockData.blockIndices.push_back(value);
	}
}

void Block::Clear()
{
	m_VertexOffset = 0;
	m_BlockData.blockVertices.clear();
	m_BlockData.blockIndices.clear();
	m_UniqevertId.clear();
}