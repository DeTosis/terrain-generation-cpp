#include "Chunk.h"
#include <iostream>

Chunk::Chunk()
{
	std::fill_n(&m_Terrain[0][0][0], ChunkSizeXY * ChunkHeight * ChunkSizeXY, BlockType::Air);
}

size_t Chunk::GetTerrainIndex(int x, int y, int z)
{
	return (x + y * ChunkSizeXY + z * ChunkSizeXY * ChunkSizeXY);
}

void Chunk::GenerateTerrain(FastNoiseLite& noise,int x, int y)
{
	if (state != ChunkState::Undefined)
		return;

	SetWorldPosition(x, y);
	CreateMesh(noise);
	state = ChunkState::TerrainGenerated;
}

void Chunk::GenerateMesh()
{
	if (state != ChunkState::TerrainGenerated)
		return;

	GenerateBlocks();

	m_Mesh.vboLayout.size = m_Mesh.vertices.size() * sizeof(float);
	m_Mesh.iboLayout.size = m_Mesh.indices.size() * sizeof(unsigned int);

	state = ChunkState::MeshGenerated;
}

void Chunk::SetNeighbours(NeighbourChunks neighbours)
{
	m_Neighbours = neighbours;
}

void Chunk::AllocateChunk(VertexBuffer& vb, IndexBuffer& ib)
{
	if (state != ChunkState::MeshGenerated)
		return;

	int hresult = -1;
	vb.Bind();
	m_Mesh.vboLayout.offset = vb.Allocate(
			m_Mesh.vertices.data(), m_Mesh.vboLayout.size);
	
	ib.Bind();
	m_Mesh.iboLayout.offset = ib.Allocate(
		m_Mesh.indices.data(), m_Mesh.iboLayout.size);

	if (m_Mesh.iboLayout.offset >= 0 && m_Mesh.vboLayout.offset >= 0)
		state = ChunkState::ChunkAllocated;
}

void Chunk::Deallocate(VertexBuffer& vb, IndexBuffer& ib)
{
	if (state != ChunkState::ChunkAllocated)
		return;

	vb.Free(m_Mesh.vboLayout.offset, m_Mesh.vboLayout.size);
	ib.Free(m_Mesh.iboLayout.offset, m_Mesh.iboLayout.size);

	m_Mesh.vboLayout.offset = -1;
	m_Mesh.iboLayout.offset = -1;

	state = ChunkState::MeshGenerated;
}

void Chunk::CreateMesh(FastNoiseLite& noise)
{
	float heightMap[ChunkSizeXY][ChunkSizeXY];

	for (int x = 0; x < ChunkSizeXY; x++)
	{
		for (int y = 0; y < ChunkSizeXY; y++)
		{
			float worldX = m_WorldX * static_cast<float>(ChunkSizeXY) + x;
			float worldY = m_WorldY * static_cast<float>(ChunkSizeXY) + y;

			float noiseValue = noise.GetNoise(worldX, worldY);
			float normalized = (noiseValue + 1.0f) / 2.0f;

			heightMap[x][y] = normalized * ChunkHeight / 2;
		}
	}

	for (int x = 0; x < ChunkSizeXY; x++)
	{
		for (int y = 0; y < ChunkSizeXY; y++)
		{
			int height = static_cast<int>(heightMap[x][y]);
			for (int z = 0; z < ChunkHeight; z++)
			{
				if (z < height)
					m_Terrain[x][z][y] = BlockType::Undefined;

				if (z > ChunkHeight / 2)
					m_Terrain[x][z][y] = BlockType::Air;
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


	Block block;
	for (int x = 0; x < ChunkSizeXY; x++)
	{
		for (int z = 0; z < ChunkHeight; z++)
		{
			for (int y = 0; y < ChunkSizeXY; y++)
			{
				block.SetInChunkPosition(
					{ (int)(x + ChunkSizeXY * m_WorldX),
					  (int)(z), 
					  (int)(y + ChunkSizeXY * m_WorldY)
					});

				if (m_Terrain[x][z][y] == BlockType::Air)
					continue;

				if (x == 0 || m_Terrain[x - 1][z][y] == BlockType::Air)
					block.AppendFace(Face::LEFT);

				if (x + 1 == ChunkSizeXY || m_Terrain[x + 1][z][y] == BlockType::Air)
					block.AppendFace(Face::RIGHT);

				if (y == 0 || m_Terrain[x][z][y - 1] == BlockType::Air)
					block.AppendFace(Face::BACK);

				if (y + 1 == ChunkSizeXY || m_Terrain[x][z][y + 1] == BlockType::Air)
					block.AppendFace(Face::FRONT);

				if (z == 0 || m_Terrain[x][z - 1][y] == BlockType::Air)
					block.AppendFace(Face::BOTTOM);

				if (z + 1 == ChunkHeight || m_Terrain[x][z + 1][y] == BlockType::Air)
					block.AppendFace(Face::TOP);

				block.m_VertexOffset = m_Mesh.vertices.size() / m_VertexSize;

				block.BuldMesh();

				m_Mesh.vertices.insert(m_Mesh.vertices.end(), 
					block.m_BlockData.blockVertices.begin(), block.m_BlockData.blockVertices.end());
				m_Mesh.indices.insert(m_Mesh.indices.end(), 
					block.m_BlockData.blockIndices.begin(), block.m_BlockData.blockIndices.end());
				
				block.Clear();
			}
		}
	}


}

