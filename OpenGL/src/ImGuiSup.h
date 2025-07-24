#pragma once
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class ImGuiSup
{
public:
	ImGuiSup(GLFWwindow* window);
	void NewFrame() const;
	void DrawCall() const;
};