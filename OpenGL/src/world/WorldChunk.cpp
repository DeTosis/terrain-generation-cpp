#include "WorldChunk.h"

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

	for (int x = 0; x < ChunkSizeXY; x++)
	{
		for (int y = 0; y < ChunkSizeXY; y++)
		{
			float worldX = m_WorldX * static_cast<float>(ChunkSizeXY) + x;
			float worldY = m_WorldY * static_cast<float>(ChunkSizeXY) + y;

			float noiseValue = m_Noise->GetNoise(worldX, worldY);
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
					m_Terrain.at(x + y * ChunkSizeXY + z * ChunkSizeXY * ChunkSizeXY) = BlockType::Undefined;

				if (z > ChunkHeight / 2)
					m_Terrain.at(x + y * ChunkSizeXY + z * ChunkSizeXY * ChunkSizeXY) = BlockType::Air;
			}
		}
	}

	m_ChunkState = ChunkState::TerrainGenerated;
}

void WorldChunk::GenerateMesh()
{
	if (m_ChunkState != ChunkState::TerrainGenerated)
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

				if (x == 0 || m_Terrain.at(GetIndex(x-1, y, z)) == BlockType::Air)
					block.AppendFace(Face::LEFT);

				if (x + 1 == ChunkSizeXY || m_Terrain.at(GetIndex(x + 1, y, z)) == BlockType::Air)
					block.AppendFace(Face::RIGHT);

				if (y == 0 || m_Terrain.at(GetIndex(x, y - 1, z)) == BlockType::Air)
					block.AppendFace(Face::BACK);

				if (y + 1 == ChunkSizeXY || m_Terrain.at(GetIndex(x, y + 1, z)) == BlockType::Air)
					block.AppendFace(Face::FRONT);

				if (z == 0 || m_Terrain.at(GetIndex(x, y, z - 1)) == BlockType::Air)
					block.AppendFace(Face::BOTTOM);

				if (z + 1 == ChunkHeight || m_Terrain.at(GetIndex(x, y, z + 1)) == BlockType::Air)
					block.AppendFace(Face::TOP);

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

void WorldChunk::GetTerrainAtIndex(BlockType& outType,  int x, int y, int z)
{
	if (x > ChunkSizeXY || y > ChunkSizeXY || z > ChunkHeight) 
		return;
	
	outType = m_Terrain.at(x + y * ChunkSizeXY + z * ChunkSizeXY * ChunkSizeXY);
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