#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <algorithm>

class VertexBuffer
{
private:
	struct Header
	{
		std::size_t offset;
		std::size_t size;
	};
	
	unsigned int m_RendererId;
	unsigned int m_TotalSize = 64 * 1024 * 1024;
public:
	std::vector<Header> m_FreeList;
public:
	VertexBuffer();
	~VertexBuffer();

	void Bind() const;
	void UnBind() const;
public:
	int Allocate(const void* data, const size_t& size);
	void Free(const size_t& offset, const size_t& size);
private:
	void MergeFreeBlocks();
};