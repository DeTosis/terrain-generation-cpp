#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
private:
	GLFWwindow* m_Window = nullptr;
	int m_Width, m_Height;
public:
	Window(const char* title, int height = 1280, int width = 720);
	void GlewInit();
	void SetSwapInterval(size_t interval);
	GLFWwindow* GetWindow();

public:
	void FrameBufferSizeCallback(GLFWframebuffersizefun callback);
};