#include "WorldChunk.h"
#include <iostream>

WorldChunk::WorldChunk(FastNoiseLite& noise)
	: m_Noise(&noise)
{
	m_Terrain.resize(ChunkSizeXY * ChunkSizeXY * ChunkHeight, BlockType::Air);
}

void WorldChunk::SetWorldPosition(int worldX, int worldY)
{
	m_WorldX = worldX;
	m_WorldY = worldY;
}

void WorldChunk::SetNeighbours(NeighbourChunks neighbours)
{
	if (neighbours.xNeg != nullptr && m_Neighbours.xNeg == nullptr)
		m_Neighbours.xNeg = neighbours.xNeg;

	if (neighbours.xPos != nullptr && m_Neighbours.xPos == nullptr)
		m_Neighbours.xPos = neighbours.xPos;

	if (neighbours.yNeg != nullptr && m_Neighbours.yNeg == nullptr)
		m_Neighbours.yNeg = neighbours.yNeg;

	if (neighbours.yPos != nullptr && m_Neighbours.yPos == nullptr)
		m_Neighbours.yPos = neighbours.yPos;
}

void WorldChunk::GenerateTerrain()
{
	if (m_ChunkState != ChunkState::Undefined)
		return;

	float heightMap[ChunkSizeXY][ChunkSizeXY];

	NosieSettings MountainNoise	{ 2.0f, 0.45f, 5.4f, 0.012f, 16, ChunkHeight, true, 8 };

	for (int x = 0; x < ChunkSizeXY; x++)
	{
		for (int y = 0; y < ChunkSizeXY; y++)
		{
			float worldX = m_WorldX * static_cast<float>(ChunkSizeXY) + x;
			float worldY = m_WorldY * static_cast<float>(ChunkSizeXY) + y;

			float height = m_NoiseGen.GetHeight(*m_Noise, worldX, worldY, MountainNoise);
			heightMap[x][y] = height;
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
					m_Terrain.at(x + y * ChunkSizeXY + z * ChunkSizeXY * ChunkSizeXY) = BlockType::Undefined;
				else
					m_Terrain.at(x + y * ChunkSizeXY + z * ChunkSizeXY * ChunkSizeXY) = BlockType::Air;
			}
		}
	}

	m_ChunkState = ChunkState::TerrainGenerated;
}

void WorldChunk::GenerateMesh()
{
	if (m_ChunkState != ChunkState::NeighboursGenerated)
		return;

	CreateMesh();
	m_Mesh->vboLayout.size = m_Mesh->vertices.size() * sizeof(float);
	m_Mesh->iboLayout.size = m_Mesh->indices.size() * sizeof(unsigned int);

	m_ChunkState = ChunkState::MeshGenerated;
}

void WorldChunk::CreateMesh()
{
	m_Mesh->indices.clear();
	m_Mesh->vertices.clear();
	m_Mesh->iboLayout.offset = -1;
	m_Mesh->vboLayout.offset = -1;

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

				if (m_Terrain.at(GetIndex(x,y,z)) == BlockType::Air)
					continue;

				if (x == 0 || m_Terrain.at(GetIndex(x - 1, y, z)) == BlockType::Air)
				{
					if (x == 0)
					{
						auto chunk = m_Neighbours.xNeg;
						BlockType bt;
						if (chunk->GetTerrainAtIndex(bt, ChunkSizeXY - 1, y, z) && bt == BlockType::Air)
							block.AppendFace(Face::LEFT);
					}
					else
						block.AppendFace(Face::LEFT);
				}

				if (x + 1 == ChunkSizeXY || (m_Terrain.at(GetIndex(x + 1, y, z)) == BlockType::Air))
				{
					if (x + 1 == ChunkSizeXY)
					{
						auto chunk = m_Neighbours.xPos;
						BlockType bt;
						if (chunk->GetTerrainAtIndex(bt, 0, y, z) && bt == BlockType::Air)
							block.AppendFace(Face::RIGHT);
					}
					else 
						block.AppendFace(Face::RIGHT);
				}

				if (y == 0 || m_Terrain.at(GetIndex(x, y - 1, z)) == BlockType::Air)
				{
					if (y == 0)
					{
						auto chunk = m_Neighbours.yNeg;
						BlockType bt;
						if (chunk->GetTerrainAtIndex(bt, x, ChunkSizeXY - 1, z) && bt == BlockType::Air)
							block.AppendFace(Face::BACK);
					}
					else
						block.AppendFace(Face::BACK);
				}

				if (y + 1 == ChunkSizeXY || m_Terrain.at(GetIndex(x, y + 1, z)) == BlockType::Air)
				{
					if (y + 1 == ChunkSizeXY)
					{
						auto chunk = m_Neighbours.yPos;
						BlockType bt;
						if (chunk->GetTerrainAtIndex(bt, x, 0, z) && bt == BlockType::Air)
							block.AppendFace(Face::FRONT);
					}
					else
						block.AppendFace(Face::FRONT);
				}

				if (z == 0 || m_Terrain.at(GetIndex(x, y, z - 1)) == BlockType::Air)
				{
					if (z > 0)
						block.AppendFace(Face::BOTTOM);
				}

				if (z + 1 == ChunkHeight || m_Terrain.at(GetIndex(x, y, z + 1)) == BlockType::Air)
				{
					block.AppendFace(Face::TOP);
				}
				block.m_VertexOffset = m_Mesh->vertices.size() / m_VertexSize;

				block.BuldMesh();

				m_Mesh->vertices.insert(m_Mesh->vertices.end(),
					block.m_BlockData.blockVertices.begin(), block.m_BlockData.blockVertices.end());
				m_Mesh->indices.insert(m_Mesh->indices.end(),
					block.m_BlockData.blockIndices.begin(), block.m_BlockData.blockIndices.end());

				block.Clear();
			}
		}
	}
}

bool WorldChunk::GetTerrainAtIndex(BlockType& outType,  int x, int y, int z)
{
	if (x > ChunkSizeXY || y > ChunkSizeXY || z > ChunkHeight) 
		return false;
	
	outType = m_Terrain.at(x + y * ChunkSizeXY + z * ChunkSizeXY * ChunkSizeXY);
	return true;
}

int WorldChunk::GetIndex(int x, int y, int z) const
{
	if (x > ChunkSizeXY || y > ChunkSizeXY || z > ChunkHeight)
		return -1;
	
	return x + y * ChunkSizeXY + z * ChunkSizeXY * ChunkSizeXY;
}

std::vector<BlockType>* WorldChunk::GetTerrain()
{
	return  &m_Terrain;
}

ChunkMesh* WorldChunk::GetMesh() const
{
	return m_Mesh;
}

ChunkState WorldChunk::GetState() const
{
	return m_ChunkState;
}

void WorldChunk::SetState(ChunkState state)
{
	m_ChunkState = ChunkState::NeighboursGenerated;
}

void WorldChunk::AllocateChunk(VertexBuffer& vb, IndexBuffer& ib)
{
	if (m_ChunkState != ChunkState::MeshGenerated)
		return;

	int hresult = -1;
	vb.Bind();
	m_Mesh->vboLayout.offset = vb.Allocate(
		m_Mesh->vertices.data(), m_Mesh->vboLayout.size);

	ib.Bind();
	m_Mesh->iboLayout.offset = ib.Allocate(
		m_Mesh->indices.data(), m_Mesh->iboLayout.size);

	if (m_Mesh->iboLayout.offset >= 0 && m_Mesh->vboLayout.offset >= 0)
		m_ChunkState = ChunkState::ChunkAllocated;
}

void WorldChunk::DeallocateChunk(VertexBuffer& vb, IndexBuffer& ib)
{
	if (m_ChunkState != ChunkState::ChunkAllocated)
		return;

	vb.Free(m_Mesh->vboLayout.offset, m_Mesh->vboLayout.size);
	ib.Free(m_Mesh->iboLayout.offset, m_Mesh->iboLayout.size);

	m_Mesh->vboLayout.offset = -1;
	m_Mesh->iboLayout.offset = -1;

	m_ChunkState = ChunkState::MeshGenerated;
}

bool WorldChunk::IsEmptyNeighbours()
{
	return
		m_Neighbours.xNeg == nullptr || 
		m_Neighbours.xPos == nullptr || 
		m_Neighbours.yNeg == nullptr || 
		m_Neighbours.yPos == nullptr;
}
