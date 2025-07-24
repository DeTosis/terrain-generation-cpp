#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class IndexBuffer
{
private:
	unsigned int m_RendererId;
public:
	IndexBuffer(void* data, unsigned int size);
	~IndexBuffer();

	void Bind() const;
	void UnBind() const;
};