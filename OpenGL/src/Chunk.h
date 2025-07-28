#pragma once

#include <vector>

#include "Block.h"
#include <fastNoise/FastNoiseLite.h>

struct ChunkData
{
	std::vector<float> vert;
	std::vector<unsigned int> indi;
};

const static unsigned int m_ChunkSize = 16;
class Chunk
{
public:
	BlockType m_Chunk[m_ChunkSize][m_ChunkSize][m_ChunkSize];
private:
	int m_WorldX = 0;
	int m_WorldY = 0;

public:
	Chunk();

	void GenerateTerrain(FastNoiseLite& noise);
	void SetWorldPosition(int x, int y);

	void GenerateBlocks(std::vector<float>& vert, std::vector<unsigned int>& indi);
};