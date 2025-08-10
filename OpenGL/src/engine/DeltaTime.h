#pragma once
#include <GLFW/glfw3.h>

static class DeltaTime
{
private:
	float m_LastFrame;
public:
	float deltaTime;
	inline void UpdateDeltaTime()
	{
		float current = (float)glfwGetTime();
		deltaTime = current - m_LastFrame;
		m_LastFrame = current;
	}
};