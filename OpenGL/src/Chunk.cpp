#include "Chunk.h"

Chunk::Chunk()
{
	std::fill_n(&m_Chunk[0][0][0], m_ChunkSize * m_ChunkSize * m_ChunkSize, BlockType::AIR);
}

void Chunk::GenerateTerrain(FastNoiseLite& noise)
{
	float heightMap[m_ChunkSize][m_ChunkSize];

	for (int x = 0; x < m_ChunkSize; x++)
	{
		for (int y = 0; y < m_ChunkSize; y++)
		{
			float worldX = x + m_WorldX * m_ChunkSize;
			float worldY = y + m_WorldY * m_ChunkSize;

			float noiseValue = noise.GetNoise(worldX, worldY);
			float normalized = (noiseValue + 1.0f) / 2.0f;

			heightMap[x][y] = normalized * (float)m_ChunkSize;
		}
	}

	for (int x = 0; x < m_ChunkSize; x++)
	{
		for (int y = 0; y < m_ChunkSize; y++)
		{
			int height = static_cast<int>(heightMap[x][y]);
			for (int z = 0; z < m_ChunkSize; z++)
			{
				if (z < height)
					m_Chunk[x][z][y] = BlockType::DEV;
				else
					m_Chunk[x][z][y] = BlockType::AIR;
			}
		}
	}
}

void Chunk::SetWorldPosition(int x, int y)
{
	m_WorldX = x;
	m_WorldY = y;
}

void Chunk::GenerateBlocks(std::vector<float>& vert, std::vector<unsigned int>& indi)
{
	for (int x = 0; x < m_ChunkSize; x++)
	{
		for (int z = 0; z < m_ChunkSize; z++)
		{
			for (int y = 0; y < m_ChunkSize; y++)
			{
				Block block;

				block.SetInChunkPosition(x + m_ChunkSize * m_WorldX, z, y + m_ChunkSize * m_WorldY);

				if (m_Chunk[x][z][y] == BlockType::AIR)
					continue;

				if (x == 0 || m_Chunk[x - 1][z][y] == BlockType::AIR)
					block.AddFace(Block::Face::LEFT);

				if (x + 1 == m_ChunkSize || m_Chunk[x + 1][z][y] == BlockType::AIR)
					block.AddFace(Block::Face::RIGHT);

				if (y == 0 || m_Chunk[x][z][y - 1] == BlockType::AIR)
					block.AddFace(Block::Face::BACK);

				if (y + 1 == m_ChunkSize || m_Chunk[x][z][y + 1] == BlockType::AIR)
					block.AddFace(Block::Face::FRONT);

				if (z == 0 || m_Chunk[x][z - 1][y] == BlockType::AIR)
					block.AddFace(Block::Face::BOTTOM);

				if (z + 1 == m_ChunkSize || m_Chunk[x][z + 1][y] == BlockType::AIR)
					block.AddFace(Block::Face::TOP);

				block.m_VertexOffset = vert.size() / 6;
				block.Assemble();

				vert.insert(vert.end(), block.m_Vertices.begin(), block.m_Vertices.end());
				indi.insert(indi.end(), block.m_Indices.begin(), block.m_Indices.end());
			}
		}
	}
}
