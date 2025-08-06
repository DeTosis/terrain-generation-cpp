#pragma once
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui/imgui.h"

class Camera
{
private:
	float m_LastX = 0;
	float m_LastY = 0;

	float m_Sensitivity;
	float m_CameraSpeed;

	bool m_MouseCaptured;
	bool m_FirstClick = true;
	bool m_SpeedBoosted = false;
public:
	float m_Yaw = -90.0f;
	float m_Pitch = 0.0f;

	glm::vec3 m_CameraFront = glm::vec3({0});
	glm::vec3 m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_CameraPos = glm::vec3(0.0f, 24.0f, 3.0f);
	Camera(const float sens, const float speed);
	
	glm::mat4 CameraLookMatrix(GLFWwindow* window);
	void Move(GLFWwindow* window, float& deltaTime);

	void UpdateCursorLockState(GLFWwindow* window);
	bool GetMouseState();
	void SetMouseState(bool state);
};