#pragma once
#include <vector>

inline constexpr float CubeVerticesSet[8][6]
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

inline constexpr unsigned int IndicesBackFace[6]{ 0, 1, 2, 2, 3, 0 };
inline constexpr unsigned int IndicesFrontFace[6]{ 4, 5, 6, 6, 7, 4 };
inline constexpr unsigned int IndicesLeftFace[6]{ 0, 4, 7, 7, 3, 0 };
inline constexpr unsigned int IndicesRightFace[6]{ 1, 5, 6, 6, 2, 1 };
inline constexpr unsigned int IndicesTopFace[6]{ 3, 2, 6, 6, 7, 3 };
inline constexpr unsigned int IndicesBottomFace[6]{ 0, 1, 5, 5, 4, 0 };

enum class BlockType : uint8_t
{
	Undefined,
	Air,
};

enum class Face
{
	BACK = 0,
	FRONT,
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
	ALL,
};

struct BlockLocalPosition
{
	int x, y, z;
	BlockLocalPosition(int x = 0, int y = 0, int z = 0)
		: x(x), y(y), z(z)
	{}
};

struct BlockData
{
	// MESH
	std::vector<float> blockVertices;
	std::vector<unsigned int> blockIndices;
	// DATA
	BlockType blockType;
};