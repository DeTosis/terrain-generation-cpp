#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class VertexBuffer
{
private:
	unsigned int m_RendererId;
public:
	VertexBuffer(void* data, unsigned int size);
	~VertexBuffer();

	void Bind() const;
	void UnBind() const;
};