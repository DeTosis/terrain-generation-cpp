#include "Block.h"

Block::Block()
{
	m_ChunkRelPos[0] = 0;
	m_ChunkRelPos[1] = 0;
	m_ChunkRelPos[2] = 0;
}

unsigned int Block::GetSize(DataType type) const
{
	if (type == DataType::VERTICES)
		return m_Vertices.size() * sizeof(float);
	else
		return m_Indices.size() * sizeof(unsigned int);
}
float* Block::GetVertices() { return m_Vertices.data(); }
unsigned int* Block::GetIndices() { return m_Indices.data(); }
void Block::AddFace(Face face)
{
	switch (face)
	{
	case Face::BACK:
		AddIndices(m_IBackFace);
		break;
	case Face::FRONT:
		AddIndices(m_IFrontFace);
		break;
	case Face::LEFT:
		AddIndices(m_ILeftFace);
		break;
	case Face::RIGHT:
		AddIndices(m_IRightFace);
		break;
	case Face::TOP:
		AddIndices(m_ITopFace);
		break;
	case Face::BOTTOM:
		AddIndices(m_IBottomFace);
		break;
	case Face::ALL:
		AddFace(Face::BACK);
		AddFace(Face::FRONT);
		AddFace(Face::LEFT);
		AddFace(Face::RIGHT);
		AddFace(Face::TOP);
		AddFace(Face::BOTTOM);
		break;
	}
}
void Block::Assemble()
{
	std::sort(m_UniqevertId.begin(), m_UniqevertId.end());
	m_UniqevertId.erase(std::unique(m_UniqevertId.begin(), m_UniqevertId.end()), m_UniqevertId.end());

	std::unordered_map<unsigned int, unsigned int> idRemap;
	for (int i = 0; i < m_UniqevertId.size(); i++)
	{
		unsigned int oldId = m_UniqevertId[i];
		idRemap[oldId] = i;

		m_Vertices.push_back(m_VerticesSet[oldId][0] + m_ChunkRelPos[0]);
		m_Vertices.push_back(m_VerticesSet[oldId][1] + m_ChunkRelPos[1]);
		m_Vertices.push_back(m_VerticesSet[oldId][2] + m_ChunkRelPos[2]);

		m_Vertices.push_back(m_VerticesSet[oldId][3]);
		m_Vertices.push_back(m_VerticesSet[oldId][4]);
		m_Vertices.push_back(m_VerticesSet[oldId][5]);
	}

	for (int i = 0; i < m_Indices.size(); i++)
	{
		m_Indices[i] = idRemap[m_Indices[i]] + m_VertexOffset;
	}
}

void Block::SetInChunkPosition(const int& x, const int& z, const int& y)
{
	int temp[3] = { x, z, y };
	std::memcpy(m_ChunkRelPos, temp, sizeof(m_ChunkRelPos));
}

void Block::AddIndices(const unsigned int indicesSet[])
{
	for (int i = 0; i < 6; i++)
	{
		auto value = indicesSet[i];
		m_UniqevertId.push_back(value);
		m_Indices.push_back(value);
	}
}