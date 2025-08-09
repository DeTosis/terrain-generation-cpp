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
		if (!IsChunkLoaded(x, y))
		{
			GenerateChunk(x, y);
		}
	}
}

void WorldGeneration::PrepareChunksForDraw(VertexBuffer& vb, IndexBuffer& ib)
{
	m_ChunksToRender.clear();
	for (const auto& it : m_LoadedChunks)
	{
		auto& chunk = it.chunk;

		auto elem = std::find(
			m_ChunksInRenderDistance.begin(), m_ChunksInRenderDistance.end(), it.coords);

		auto state = it.chunk->GetState();
		if (elem != m_ChunksInRenderDistance.end())
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

void WorldGeneration::GenerateChunk(int worldX, int worldY)
{
	if (IsChunkLoaded(worldX, worldY)) return;

	WorldChunk* chunk = new WorldChunk(m_worldNoise);

	chunk->SetWorldPosition(worldX, worldY);
	chunk->GenerateTerrain();
	chunk->GenerateMesh();

	m_LoadedChunks.push_back({ {worldX, worldY}, std::move(chunk) });
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

bool WorldGeneration::IsChunkLoaded(int worldX, int worldY)
{
	std::pair<int, int> coords = { worldX, worldY };
	auto it = std::find_if(m_LoadedChunks.begin(), m_LoadedChunks.end(), [&](const ChunksMap& chunk)
		{
			return chunk.coords == coords;
		});

	return it != m_LoadedChunks.end();
}

WorldChunk* WorldGeneration::GetChunk(int worldX, int worldY)
{
	if (!IsChunkLoaded(worldX, worldY)) return nullptr;

	std::pair<int, int> coords = { worldX, worldY };
	auto it = std::find_if(m_LoadedChunks.begin(), m_LoadedChunks.end(), [&](const ChunksMap& chunk)
		{
			return chunk.coords == coords;
		});

	return it->chunk;
}
