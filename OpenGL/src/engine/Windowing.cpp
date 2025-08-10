#include "Windowing.h"

Window::Window(const char* title, int height, int width)
	: m_Height(height), m_Width(width)
{
	if (!glfwInit())
		m_Window = nullptr;

	m_Window = glfwCreateWindow(m_Height, m_Width, title, nullptr, nullptr);
	if (!m_Window)
	{
		glfwTerminate();
		m_Window = nullptr;
	}
	glfwMakeContextCurrent(m_Window);
}

void Window::GlewInit()
{
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		m_Window = nullptr;
	}
}

void Window::SetSwapInterval(size_t interval)
{
	glfwSwapInterval(interval);
}

GLFWwindow* Window::GetWindow()
{
	return m_Window;
}

void Window::FrameBufferSizeCallback(GLFWframebuffersizefun callback)
{
	glfwSetFramebufferSizeCallback(m_Window, callback);
}

