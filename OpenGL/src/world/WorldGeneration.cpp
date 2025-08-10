#include "WorldGeneration.h"

std::mutex mtx;
static void ManageGenaration(std::vector<WorldChunk*>& generationQueue);

WorldGeneration::WorldGeneration(int seed)
{
	GenerateNoise(seed);
	std::thread generator(ManageGenaration, std::ref(m_GenerationQueue));
	m_GenerationThread = std::move(generator);
}

WorldGeneration::~WorldGeneration()
{
	m_GenerationThread.join();
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
		auto [x, y] = chunk.coord;
		if (!m_LoadedChunks.contains({ x,y }))
			PreGenerateChunk(x, y);
		m_ChunksInRenderDistance.insert({x,y});
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

		if (m_ChunksInRenderDistance.contains({x,y}))
		{
			if (m_LoadedChunks.contains({ x,y }))
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

void WorldGeneration::PostGenerateChunks()
{
	for (const auto& it : m_LoadedChunks)
	{
		auto& chunk = it.second;
		auto& [x, y] = it.first;

		auto state = chunk->GetState();

		if (state == ChunkState::TerrainGenerated && !chunk->IsEmptyNeighbours())
		{
			chunk->SetState(ChunkState::Queued);
			m_GenerationQueue.push_back(chunk);
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

static void ThreadGenerateChunk(std::promise<void> p, WorldChunk* chunk)
{
	chunk->SetState(ChunkState::NeighboursGenerated);
	chunk->GenerateMesh();

	p.set_value();
}


static void ManageGenaration(std::vector<WorldChunk*>& generationQueue)
{
	int threadLimit = 4;
	std::vector<std::pair<std::future<void>, std::thread>> workingThreads;
	while (true)
	{
		std::lock_guard lock(mtx);
		{
			if (generationQueue.empty())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			while (workingThreads.size() < threadLimit)
			{
				if(generationQueue.empty())
					break;

				std::promise<void> p;
				std::future<void> f = p.get_future();

				WorldChunk* chunk = nullptr;
				chunk = generationQueue.front();

				std::thread t(ThreadGenerateChunk, std::move(p), std::move(chunk));
				workingThreads.push_back({ std::move(f), std::move(t) });

				generationQueue.erase(generationQueue.begin());
			}
			
			for (auto it = workingThreads.begin(); it != workingThreads.end();)
			{
				auto& [f, t] = *it;

				if (f.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
				{
					if (t.joinable())
					{
						t.join();
						it = workingThreads.erase(it);
					}
				}
			}
		}
	}
}