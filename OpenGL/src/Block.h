#pragma once
#include <vector>

#include <algorithm>
#include <unordered_map>

class Block
{
private:
	float m_VerticesSet[8][6]
	{
		{ -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f },
		{  0.5f, -0.5f, -0.5f, 0.0f, 0.5f, 0.0f },
		{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f },
		{ -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f },
		{ -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f },
		{  0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f },
		{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f },
		{ -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f },
	};

	unsigned int	m_IBackFace[6]	{ 0, 1, 2, 2, 3, 0 };
	unsigned int	m_IFrontFace[6]	{ 4, 5, 6, 6, 7, 4 };
	unsigned int	m_ILeftFace[6]	{ 0, 4, 7, 7, 3, 0 };
	unsigned int	m_IRightFace[6]	{ 1, 5, 6, 6, 2, 1 };
	unsigned int	m_ITopFace[6]	{ 3, 2, 6, 6, 7, 3 };
	unsigned int	m_IBottomFace[6]{ 0, 1, 5, 5, 4, 0 };

	std::vector<unsigned int> m_UniqevertId;
	int m_ChunkRelPos[3];
public:
	std::vector<float> m_Vertices;
	std::vector<unsigned int> m_Indices;
	int m_VertexOffset = 0;

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
	Block();
	unsigned int GetSize(DataType type) const;
	float* GetVertices();
	unsigned int* GetIndices();
	void AddFace(Face face);
	void Assemble();

	void SetInChunkPosition(const int& x, const int& z, const int& y);
private:
	void AddIndices(unsigned int indicesSet[]);
};
