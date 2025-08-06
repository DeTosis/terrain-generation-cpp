#include "Chunk.h"
#include <iostream>

Chunk::Chunk()
{
	std::fill_n(&m_Chunk[0][0][0], m_ChunkSize * m_ChunkHeight * m_ChunkSize, BlockType::AIR);
}

void Chunk::GenerateChunk(FastNoiseLite& noise,int x, int y)
{
	if (state != RenderState::NONE)
	{
		exit(-1);
	}

	SetWorldPosition(x, y);
	GenerateTerrain(noise);
	GenerateBlocks();

	m_VBOLayout.size = m_MeshData.vertices.size() * sizeof(float);
	m_IBOLayout.size = m_MeshData.indices.size() * sizeof(unsigned int);
	
	state = RenderState::GENERATED;
}

void Chunk::AllocateChunk(VertexBuffer& vb, IndexBuffer& ib, int x, int y)
{
	if (state != RenderState::GENERATED)
		return;

	vb.Bind();
	m_VBOLayout.offset = vb.Allocate(
			m_MeshData.vertices.data(), m_VBOLayout.size);

	ib.Bind();
	m_IBOLayout.offset = ib.Allocate(
			m_MeshData.indices.data(), m_IBOLayout.size);

	state = RenderState::ALLOCATED;
}

void Chunk::GenerateTerrain(FastNoiseLite& noise)
{
	float heightMap[m_ChunkSize][m_ChunkSize];

	for (int x = 0; x < m_ChunkSize; x++)
	{
		for (int y = 0; y < m_ChunkSize; y++)
		{
			float worldX = m_WorldX * static_cast<float>(m_ChunkSize) + x;
			float worldY = m_WorldY * static_cast<float>(m_ChunkSize) + y;

			float noiseValue = noise.GetNoise(worldX, worldY);
			float normalized = (noiseValue + 1.0f) / 2.0f;

			heightMap[x][y] = normalized * m_ChunkHeight - 1.0f;
		}
	}

	for (int x = 0; x < m_ChunkSize; x++)
	{
		for (int y = 0; y < m_ChunkSize; y++)
		{
			int height = static_cast<int>(heightMap[x][y]);
			for (int z = 0; z < m_ChunkHeight; z++)
			{
				if (z < height)
					m_Chunk[x][z][y] = BlockType::DEV;
			}
		}
	}
}

void Chunk::SetWorldPosition(int x, int y)
{
	m_WorldX = x;
	m_WorldY = y;
}

void Chunk::GenerateBlocks()
{
	for (int x = 0; x < m_ChunkSize; x++)
	{
		for (int z = 0; z < m_ChunkHeight; z++)
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

				if (z + 1 == m_ChunkHeight || m_Chunk[x][z + 1][y] == BlockType::AIR)
					block.AddFace(Block::Face::TOP);

				block.m_VertexOffset = m_MeshData.vertices.size() / m_VertexSize;
				block.Assemble();

				m_MeshData.vertices.insert(m_MeshData.vertices.end(), block.m_Vertices.begin(), block.m_Vertices.end());
				m_MeshData.indices.insert(m_MeshData.indices.end(), block.m_Indices.begin(), block.m_Indices.end());
			}
		}
	}
}

