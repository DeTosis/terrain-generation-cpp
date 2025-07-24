#pragma once
#include <vector>

#include <algorithm>
#include <unordered_map>

class Block
{
private:
	float m_VerticesSet[8][6]
	{
		{ -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f },
		{  1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f },
		{  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f },
		{  1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
		{  1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f },
	};

	unsigned int	m_IBackFace[6]{ 0, 1, 2, 2, 3, 0 };
	unsigned int	m_IFrontFace[6]{ 4, 5, 6, 6, 7, 4 };
	unsigned int	m_ILeftFace[6]{ 0, 4, 7, 7, 3, 0 };
	unsigned int	m_IRightFace[6]{ 1, 5, 6, 6, 2, 1 };
	unsigned int	m_ITopFace[6]{ 3, 2, 6, 6, 7, 3 };
	unsigned int	m_IBottomFace[6]{ 0, 1, 5, 5, 4, 0 };

	std::vector<unsigned int> m_UniqevertId;
public:
	std::vector<float> m_Vertices;
	std::vector<unsigned int> m_Indices;

	enum class DataType
	{
		VERTICES,
		INDICES,
	};
	enum class Face
	{
		BACK,
		FRONT,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
		ALL,
	};
public:
	unsigned int GetSize(DataType type) const;
	float* GetVertices();
	unsigned int* GetIndices();
	void AddFace(Face face);
	void Assemble();
private:
	void AddIndices(unsigned int indicesSet[]);
};
