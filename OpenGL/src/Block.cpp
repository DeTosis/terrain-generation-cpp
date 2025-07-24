#include "Block.h"

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

		for (int j = 0; j < 6; j++)
		{
			m_Vertices.push_back(m_VerticesSet[oldId][j]);
		}
	}

	for (int i = 0; i < m_Indices.size(); i++)
	{
		m_Indices[i] = idRemap[m_Indices[i]];
	}
}
void Block::AddIndices(unsigned int indicesSet[])
{
	for (int i = 0; i < 6; i++)
	{
		auto value = indicesSet[i];
		m_UniqevertId.push_back(value);
		m_Indices.push_back(value);
	}
}