#include  "Camera.h"

Camera::Camera(const float sens, const float speed)
	: m_Sensitivity(sens), m_CameraSpeed(speed), m_MouseCaptured(false) { }

glm::mat4 Camera::CameraLookMatrix(GLFWwindow* window)
{
	if (m_FirstClick)
	{
		glfwSetCursorPos(window, m_LastX, m_LastY);
		m_FirstClick = false;
	}

	glm::mat4 view;
	double xPos;
	double yPos;

	glfwGetCursorPos(window, &xPos, &yPos);

	float xOffset = xPos - m_LastX;
	float yOffset = m_LastY - yPos;

	m_LastX = xPos;
	m_LastY = yPos;

	xOffset *= m_Sensitivity;
	yOffset *= m_Sensitivity;

	m_Yaw +=   xOffset;
	m_Pitch += yOffset;

	if (m_Pitch >  89.0f) m_Pitch = 89.0f;
	if (m_Pitch < -89.0f) m_Pitch = -89.0f;

	glm::vec3 direction;

	direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	direction.y = sin(glm::radians(m_Pitch));
	direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_CameraFront = glm::normalize(direction);
	
	view = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
	
	return view;
}

void Camera::Move(GLFWwindow* window, float& deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_CameraPos -= deltaTime * glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * m_CameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_CameraPos += deltaTime * glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * m_CameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		m_CameraPos += deltaTime * m_CameraUp * m_CameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		m_CameraPos -= deltaTime * m_CameraUp * m_CameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_CameraPos += deltaTime * m_CameraFront * m_CameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_CameraPos -= deltaTime * m_CameraFront * m_CameraSpeed;
	
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		if (!m_SpeedBoosted)
		{
			m_CameraSpeed *= 4;
			m_SpeedBoosted = true;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		if (m_SpeedBoosted)
		{
			m_CameraSpeed /= 4;
			m_SpeedBoosted = false;
		}
	}
}

void Camera::UpdateCursorLockState(GLFWwindow* window)
{
	bool isHovered = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
	if (!isHovered && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		m_MouseCaptured = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		m_MouseCaptured = false;
		m_FirstClick = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

bool Camera::GetMouseState()
{
	return m_MouseCaptured;
}

void Camera::SetMouseState(bool state)
{
	m_MouseCaptured = state;
}
