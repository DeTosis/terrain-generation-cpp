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
#include <chrono>
#include "Camera.h"

#pragma region DATA
float positions[]{
	//    X      Y	   Z        COLOR
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f,

		-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
};

unsigned int indices[]
{
	0, 1, 2, 2, 3, 0, // Back face
	4, 5, 6, 6, 7, 4, // Front face
	0, 4, 7, 7, 3, 0, // Left face
	1, 5, 6, 6, 2, 1, // Right face
	3, 2, 6, 6, 7, 3, // Top face
	0, 1, 5, 5, 4, 0  // Bottom face
};

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

float f[3]{ 0 };
#pragma endregion

void UpdateDeltaTime()
{
	float current = glfwGetTime();
	deltaTime = current - lastFrame;
	lastFrame = current;
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

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER,vbo); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	Shader shader("res/shaders/basic.shader");

	glm::mat4 proj = glm::perspective(80.0f, height / width, 0.1f, 100.0f);
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//IMGUI
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	ImGui::StyleColorsDark;
	
	bool wireframe = true;

	Camera camera(0.1f, 4.0f);
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		UpdateDeltaTime();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		//RE - DO
		float speed = 4.0f;
		camera.CameraMovement(window, deltaTime, speed);
		camera.UpdateCursorLockState(window);
		if (camera.GetMouseState())
		{
			double xPos;
			double yPos;

			glfwGetCursorPos(window, &xPos, &yPos);
			view = camera.CameraLookMatrix(xPos, yPos);
		}

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(f[0], f[1], f[2]));
		glm::mat4 mvp = proj * view * model;

		shader.Bind();
		shader.SetUniformMatrix4fv("u_MVP", mvp);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		//IMGUI
		{
			ImGui::Checkbox("Wireframe", &wireframe);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Delta Time %.f ms", deltaTime * 1000.0f);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	
	return 0;
}