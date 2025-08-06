#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <print>
#include "Shader.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Camera.h"
#include "ImGuiSup.h"

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Block.h"

#include "fastNoise/FastNoiseLite.h"
#include "Chunk.h"

#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/fast_trigonometry.hpp>
#include <unordered_set>

#include <thread>
#include <queue>

float deltaTime;
float lastFrame = 0.0f;

static void APIENTRY GLCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "%s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "[ OpenGL ERROR ]" : "[ OpenGL DEBUG ]"),
		type, severity, message);
}

void UpdateDeltaTime()
{
	float current = (float)glfwGetTime();
	deltaTime = current - lastFrame;
	lastFrame = current;
}

void SetWireframeMode(unsigned int mode)
{
	glPolygonMode(GL_FRONT_AND_BACK, mode);
}

glm::vec3 WorldToChunkPos(const glm::vec3& cameraPos, const unsigned int& chunkSize)
{
	return glm::vec3({
		floor(cameraPos.x / chunkSize),
		floor(cameraPos.y / chunkSize),
		floor(cameraPos.z / chunkSize)
		});
}

float height = 1280;
float width = 720;

struct PairHash
{
	std::size_t operator()(const std::pair<int, int>& p) const
	{
		return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
	}
};

/*
* 
*  MAIN 
* 
*/
static FastNoiseLite noise;

void DrawCall(const Shader& shader, const unsigned int& vao, const VertexBuffer& vb, const IndexBuffer& ib, const Chunk& chunk)
{
	if (chunk.state != RenderState::ALLOCATED)
		return;
	
	shader.Bind();
	glBindVertexArray(vao);
	vb.Bind();
	ib.Bind();

	glDrawElementsBaseVertex(
		GL_TRIANGLES, 
		chunk.m_MeshData.indices.size(),
		GL_UNSIGNED_INT, 
		(void*)(chunk.m_IBOLayout.offset),
		chunk.m_VBOLayout.offset / (6 * sizeof(float)));
}

std::vector<glm::ivec2> GetChunkPositionsInView(const int& cx, const int& cy, int& radius)
{
	std::vector<glm::ivec2> points;
	int rSquared = radius * radius;

	for (int y = cy - radius; y <= cy + radius; ++y)
	{
		for (int x = cx - radius; x <= cx + radius; ++x)
		{
			int dx = x - cx;
			int dy = y - cy;

			if (dx * dx + dy * dy <= rSquared)
			{
				points.emplace_back(x, y);
			}
		}
	}

	return points;
}

int main()
{
#pragma region INIT
	GLFWwindow* window;
	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(height, width, "?", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GLCallback, 0);
	glDebugMessageControl(
		GL_DONT_CARE,						// source
		GL_DONT_CARE,						// type
		GL_DEBUG_SEVERITY_NOTIFICATION,		// severity
		0,									// count
		nullptr,							// ids
		GL_FALSE							// enable or disable
	);
#pragma endregion
	
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	noise.SetSeed(2212313);
	noise.SetFrequency(0.01f);
	noise.SetFractalOctaves(3);
	noise.SetCellularJitter(3.0f);
	noise.SetFractalLacunarity(2.0f);

	int worldSize = 4;

	VertexBuffer vb;
	vb.Bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	IndexBuffer ib;
	ib.Bind();

	Shader shader("res/shaders/basic.shader");

	float fov = 80.0f;
	glm::mat4 proj = glm::perspective(glm::radians(fov), height / width, 0.1f, 1000.0f);
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	bool wireframe = false;

	ImGuiSup gui(window);
	Camera camera(0.1f, 10.0f);

	glm::vec3 translation { 0 };

	unsigned int query;
	glGenQueries(1, &query);

	int renderDistance = 6;

	std::unordered_map<std::pair<int, int>, Chunk, PairHash> loadedChunks;

	while (!glfwWindowShouldClose(window))
	{
		{
			// *** FRAME BOOTSTRAP ***
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			gui.NewFrame();
			// *** FRAME BOOTSTRAP ***

			UpdateDeltaTime();
			SetWireframeMode(wireframe ? GL_LINE : GL_FILL);

			// *** CAMERA ***
			camera.Move(window, deltaTime);
			camera.UpdateCursorLockState(window);
			if (camera.GetMouseState())
				view = camera.CameraLookMatrix(window);
			// *** CAMERA ***
		}


		glBeginQuery(GL_PRIMITIVES_GENERATED, query);
		shader.Bind();
		{
 			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3{0});
 			glm::mat4 mvp = proj * view * model;
 			shader.SetUniformMatrix4fv("u_MVP", mvp);

			std::unordered_set<std::pair<int,int>, PairHash> visibleChunks;
			glm::vec2 camPos2D = glm::vec2(camera.m_CameraPos.x, camera.m_CameraPos.z);
			for (int x = -renderDistance; x < renderDistance; x++)
			{
				for (int z = -renderDistance; z < renderDistance; z++)
				{

					glm::vec2 offset = glm::vec2(x,z);
					glm::vec2 worldPos = camPos2D + offset * (float)m_ChunkSize;
					glm::ivec2 chunkCoord = glm::floor(worldPos / (float)m_ChunkSize);
					
					if (glm::length(offset) > renderDistance)
					{
						continue;
					}
					visibleChunks.insert({ chunkCoord.x, chunkCoord.y });
				}
			}

			for (const auto& it : loadedChunks)
			{
				int x = it.first.first;
				int y = it.first.second;
				if (!visibleChunks.contains({x,y}))
				{
					loadedChunks[{x,y}].UnLoad(vb,ib);
				}
			}

			for (const auto& it : visibleChunks)
			{
				int x = it.first;
				int y = it.second;

				Chunk chunk;
				if (!loadedChunks.contains({ x,y }))
				{
					chunk.GenerateChunk(noise, x, y);
					loadedChunks.insert({ { x,y },chunk });
				}
				loadedChunks.at({ x,y }).AllocateChunk(vb, ib);
			}

			for (auto& it : loadedChunks)
			{
				int x = it.first.first;
				int y = it.first.second;

				DrawCall(shader, vao, vb, ib, loadedChunks[{x, y}]);
			}
		}
		glEndQuery(GL_PRIMITIVES_GENERATED);
		int primitives = 0;
		glGetQueryObjectiv(query, GL_QUERY_RESULT, &primitives);

		// *** IMGUI ***
		{
			ImGui::Checkbox("Wireframe", &wireframe);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Delta Time %.f ms", deltaTime * 1000.0f);

			ImGui::Text("Faces     : %.i", primitives / 2);
			ImGui::Text("Triangles : %.i", primitives);
			ImGui::Text("Vertices  : %.i", primitives * 3);

			ImGui::Text("Camera pos: %.f x | %.f y | %.f z",
				camera.m_CameraPos[0], camera.m_CameraPos[1], camera.m_CameraPos[2]);

			auto pos = WorldToChunkPos(camera.m_CameraPos, m_ChunkSize);

			ImGui::Text("Camera rot: %.f x | %.f y", camera.m_Pitch, camera.m_Yaw );
			ImGui::Text("Camera vec: %.f x | %.f y", cos(camera.m_Pitch), sin(camera.m_Yaw));
			ImGui::Text("Camera frn: %.f x | %.f y | %.f z", camera.m_CameraFront.x, camera.m_CameraFront.y, camera.m_CameraFront.z);
			ImGui::Text("Chunk  pos: %.f x | %.f y | %.f z", pos.x,pos.y,pos.z);

			int freeVb = 0;
			for (const auto& it : vb.m_FreeList)
			{
				freeVb += it.size;
			}
			ImGui::Text("VBO Free list %.iKb", freeVb / 1024);

			if (ImGui::Button("Reset Camera"))
			{
				camera.m_CameraPos = glm::vec3(0, 0, 3);
				camera.m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
				camera.m_CameraFront = glm::vec3({ 0 });
				camera.m_Pitch = 0.0f;
				camera.m_Yaw = -90.0f;
				view = camera.CameraLookMatrix(window);
			}
		}

		gui.DrawCall();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteQueries(1, &query);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	
	return 0;
}