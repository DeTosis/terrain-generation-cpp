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

float deltaTime;
float lastFrame = 0.0f;
#pragma endregion

#pragma region CALLBACK
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

#pragma endregion

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

float height = 640;
float width = 480;

int main()
{
#pragma region INIT
	GLFWwindow* window;
	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(640, 480, "?", nullptr, nullptr);
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

	Block block;
	block.AddFace(Block::Face::ALL);
	block.Assemble();

	VertexBuffer vb(block.GetVertices(), block.GetSize(Block::DataType::VERTICES));

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	IndexBuffer ib(block.GetIndices(), block.GetSize(Block::DataType::INDICES));

	Shader shader("res/shaders/basic.shader");

	glm::mat4 proj = glm::perspective(80.0f, height / width, 0.1f, 100.0f);
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	bool wireframe = false;

	ImGuiSup gui(window);
	Camera camera(0.1f, 4.0f);

	glm::vec3 translation { 0 };

	unsigned int query;
	glGenQueries(1, &query);

	const unsigned int chunkSize = 4;
	int chunk[chunkSize][chunkSize][chunkSize] {1};

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		UpdateDeltaTime();
		gui.NewFrame();

		SetWireframeMode(wireframe ? GL_LINE : GL_FILL);

		//RE - DO
		float speed = 4.0f;
		camera.Move(window, deltaTime, speed);
		camera.UpdateCursorLockState(window);
		if (camera.GetMouseState())
			view = camera.CameraLookMatrix(window);

		// *** DRAW CALL
		glBeginQuery(GL_PRIMITIVES_GENERATED, query);

		shader.Bind();
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3{ 0 });
			glm::mat4 mvp = proj * view * model;

			shader.SetUniformMatrix4fv("u_MVP", mvp);

			shader.Bind();
			glBindVertexArray(vao);
			vb.Bind();
			ib.Bind();
			glDrawElements(GL_TRIANGLES, block.m_Indices.size(), GL_UNSIGNED_INT, 0);
		}

		glEndQuery(GL_PRIMITIVES_GENERATED);
		int primitives = 0;

		glGetQueryObjectiv(query, GL_QUERY_RESULT, &primitives);
		// *** DRAW CALL

		//IMGUI
		{
			ImGui::Checkbox("Wireframe", &wireframe);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Delta Time %.f ms", deltaTime * 1000.0f);

			ImGui::Text("Faces     : %.i", primitives / 2);
			ImGui::Text("Triangles : %.i", primitives);
			ImGui::Text("Vertices  : %.i", primitives * 3);
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