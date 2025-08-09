#include "WorldGeneration.h"

WorldGeneration::WorldGeneration(int seed)
{
	GenerateNoise(seed);
}

void WorldGeneration::UpdateChunksInRenderDistance(int chunkRenderDistance, Camera& camera)
{
	std::vector<ChunkWithDist> m_Chunks;

	m_ChunksInRenderDistance.clear();
	glm::vec2 camPos2D = glm::vec2(camera.m_CameraPos.x, camera.m_CameraPos.z);
	
	const float chunkSizeF = static_cast<float>(ChunkSizeXY);
	for (int x = -chunkRenderDistance; x < chunkRenderDistance; x++)
	{
		for (int z = -chunkRenderDistance; z < chunkRenderDistance; z++)
		{
			glm::vec2 offset = glm::vec2(x, z);
			float dist = glm::length(offset);
			if (dist < chunkRenderDistance)
			{
				glm::vec2 worldPos = camPos2D + offset * chunkSizeF;
				glm::ivec2 chunkCoord = glm::floor(worldPos / chunkSizeF);
				m_Chunks.push_back({ {chunkCoord.x, chunkCoord.y}, dist });
			}
		}
	}

	std::sort(m_Chunks.begin(), m_Chunks.end(), [](const ChunkWithDist& a, const ChunkWithDist& b)
		{
			return a.dist < b.dist;
		});

	for (const auto& chunk : m_Chunks)
	{
		m_ChunksInRenderDistance.push_back(chunk.coord);
	}
}

void WorldGeneration::GenerateVisibleChunks()
{
	for (const auto& it : m_ChunksInRenderDistance)
	{
		auto& [x, y] = it;
		if (!m_LoadedChunks.contains({x,y}))
		{
			PreGenerateChunk(x, y);
		}
	}
}

void WorldGeneration::PrepareChunksForDraw(VertexBuffer& vb, IndexBuffer& ib)
{
	m_ChunksToRender.clear();
	for (const auto& it : m_LoadedChunks)
	{
		auto& chunk = it.second;
		auto& [x, y] = it.first;

		auto state = chunk->GetState();

		if (m_LoadedChunks.contains({x,y}))
		{
			if (state == ChunkState::MeshGenerated)
			{
				chunk->AllocateChunk(vb, ib);
			}
			else if (state == ChunkState::ChunkAllocated)
			{
				m_ChunksToRender.push_back(chunk);
			}
		}
		else
		{
			if (state == ChunkState::ChunkAllocated)
			{
				chunk->DeallocateChunk(vb, ib);
			}
		}
	}
}

void WorldGeneration::UpdateChunks()
{
	for (const auto& it : m_LoadedChunks)
	{
		auto& chunk = it.second;
		auto& [x, y] = it.first;

		auto state = chunk->GetState();

		if (state == ChunkState::TerrainGenerated && !chunk->IsEmptyNeighbours())
		{
			chunk->SetState(ChunkState::NeighboursGenerated);
			chunk->GenerateMesh();
		}
		else if (chunk->IsEmptyNeighbours())
		{
			UpdateNeighbours(chunk);
		}
		else
		{
			// "Dirty chunks update time"
		}
	}
}

void WorldGeneration::PreGenerateChunk(int worldX, int worldY)
{
	if (m_LoadedChunks.contains({ worldX, worldY })) return;

	WorldChunk* chunk = new WorldChunk(m_worldNoise);

	chunk->SetWorldPosition(worldX, worldY);

	UpdateNeighbours(chunk);

	chunk->GenerateTerrain();
	chunk->GenerateMesh();

	m_LoadedChunks.insert({ {worldX, worldY}, std::move(chunk) });
}

void WorldGeneration::UpdateNeighbours(WorldChunk* chunk)
{
	int worldX = chunk->m_WorldX;
	int worldY = chunk->m_WorldY;

	NeighbourChunks nb;

	nb.xNeg = GetChunk(worldX - 1, worldY);
	nb.xPos = GetChunk(worldX + 1, worldY);
	nb.yNeg = GetChunk(worldX, worldY - 1);
	nb.yPos = GetChunk(worldX, worldY + 1);

	chunk->SetNeighbours(nb);
}

void WorldGeneration::AllocateChunk(VertexBuffer& vb, IndexBuffer& ib,  int worldX, int worldY)
{
	WorldChunk* chunk = GetChunk(worldX, worldY);
	if (chunk != nullptr)
		chunk->AllocateChunk(vb,ib);
}

void WorldGeneration::DeallocateChunk(VertexBuffer& vb, IndexBuffer& ib, int worldX, int worldY)
{
	WorldChunk* chunk = GetChunk(worldX, worldY);
	if (chunk != nullptr)
		chunk->DeallocateChunk(vb, ib);
}


void WorldGeneration::GenerateNoise(int seed)
{
	m_worldNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

	m_worldNoise.SetSeed(seed);
	m_worldNoise.SetFrequency(0.01f);
}

WorldChunk* WorldGeneration::GetChunk(int worldX, int worldY)
{
	if (!m_LoadedChunks.contains({worldX, worldY})) return nullptr;

	return m_LoadedChunks.at({worldX, worldY});
}