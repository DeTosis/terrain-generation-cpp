#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <unordered_map>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Shader.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

#include "ImGuiSup.h"
#include "Camera.h"

#include "Block.h"
#include "world/WorldChunk.h"

#include "winApi/Memory.h"
#include "world/WorldGeneration.h"

float deltaTime;
float lastFrame = 0.0f;

void UpdateDeltaTime()
{
	float current = (float)glfwGetTime();
	deltaTime = current - lastFrame;
	lastFrame = current;
}

void SetWireframeMode(unsigned int mode) { glPolygonMode(GL_FRONT_AND_BACK, mode); }
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


void DrawCall(
	const Shader& shader, 
	const unsigned int& vao, 
	const VertexBuffer& vb, const IndexBuffer& ib, 
	const WorldChunk* chunk)
{
	if (chunk->GetState() != ChunkState::ChunkAllocated)
		return;
	
	shader.Bind();
	glBindVertexArray(vao);
	vb.Bind();
	ib.Bind();

	auto mesh = chunk->GetMesh();

	glDrawElementsBaseVertex(
		GL_TRIANGLES, 
		mesh->indices.size(),
		GL_UNSIGNED_INT, 
		(void*)(mesh->iboLayout.offset),
		mesh->vboLayout.offset / (6 * sizeof(float)));
}

int main()
{
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

	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

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
	view = camera.CameraLookMatrix(window);

	unsigned int query;
	glGenQueries(1, &query);

	int renderDistance = 4;
	WorldGeneration world(1488);
	while (!glfwWindowShouldClose(window))
	{
		{
			// *** FRAME BOOTSTRAP ***
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0.439f, 0.675f, 0.851f, 1.0f);
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
		{
			shader.Bind();
 			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3{0});
 			glm::mat4 mvp = proj * view * model;
 			shader.SetUniformMatrix4fv("u_MVP", mvp);

			world.UpdateChunksInRenderDistance(renderDistance, camera);
			world.GenerateVisibleChunks();

			world.UpdateChunks();
			
			world.PrepareChunksForDraw(vb, ib);

			for (const auto& it : world.m_ChunksToRender)
			{
				DrawCall(shader, vao, vb, ib, it);
			}
		}
		glEndQuery(GL_PRIMITIVES_GENERATED);
		int primitives = 0;
		glGetQueryObjectiv(query, GL_QUERY_RESULT, &primitives);

		// *** IMGUI ***
		{
			ImGui::Text("Render Distance");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(60);
			ImGui::SliderInt("##.", &renderDistance, 1, 32);

			ImGui::SameLine();
			if (ImGui::Button("+"))
				renderDistance++;
			ImGui::SameLine();
			if (ImGui::Button("-"))
				renderDistance--;

			ImGui::Checkbox("Wireframe", &wireframe);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Delta Time %.f ms", deltaTime * 1000.0f);

			ImGui::Text("Faces     : %.i", primitives / 2);
			ImGui::Text("Triangles : %.i", primitives);
			ImGui::Text("Vertices  : %.i", primitives * 3);

			ImGui::Text("Camera pos: %.f x | %.f y | %.f z",
				camera.m_CameraPos[0], camera.m_CameraPos[1], camera.m_CameraPos[2]);

			auto pos = WorldToChunkPos(camera.m_CameraPos, ChunkSizeXY);

			ImGui::Text("Camera rot: %.f x | %.f y", camera.m_Pitch, camera.m_Yaw );
			ImGui::Text("Camera vec: %.f x | %.f y", cos(camera.m_Pitch), sin(camera.m_Yaw));
			ImGui::Text("Camera frn: %.f x | %.f y | %.f z", camera.m_CameraFront.x, camera.m_CameraFront.y, camera.m_CameraFront.z);
			ImGui::Text("Chunk  pos: %.f x | %.f y | %.f z", pos.x,pos.y,pos.z);

			ImGui::Text("Memory usage [%d] Mb", Memory::GetCurrentProcessUsage() / (1024 * 1024));

			int freeVb = 0;
			for (const auto& it : vb.m_FreeList)
			{
				freeVb += it.size;
			}
			ImGui::Text("VBO Free list %.iKb", freeVb / 1024);

			if (ImGui::Button("Reset Camera"))
			{
				camera.m_CameraPos = glm::vec3(0, 24, 3);
				camera.m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
				camera.m_CameraFront = glm::vec3({ 0 });
				camera.m_Pitch = 0.0f;
				camera.m_Yaw = -90.0f;
				view = camera.CameraLookMatrix(window);
			}
		}
		// *** IMGUI ***

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
