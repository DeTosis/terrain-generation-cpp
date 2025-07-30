#include "IndexBuffer.h"

IndexBuffer::IndexBuffer()
{
	m_FreeList.push_back({0, m_TotalSize});
	glGenBuffers(1, &m_RendererId);
	Bind();
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER, 
		m_TotalSize, 
		nullptr, 
		GL_DYNAMIC_DRAW
	);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_RendererId);
}

void IndexBuffer::Bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
};

void IndexBuffer::UnBind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
};

int IndexBuffer::Allocate(const void* data, const size_t& size)
{
	Bind();
	size_t offset;
	for (auto it = m_FreeList.begin(); it != m_FreeList.end(); it++)
	{
		if (it->size >= size)
		{
			offset = it->offset;
			if (it->size == size)
			{
				offset = it->offset;
				m_FreeList.erase(it);
			}
			else
			{
				it->offset += size;
				it->size -= size;
			}

			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
			return static_cast<int>(offset);
		}
	}
	return -1;
}

void IndexBuffer::Free(const size_t& offset, const size_t& size)
{
	m_FreeList.push_back({ offset, size });
	MergeFreeBlocks();
}

void IndexBuffer::MergeFreeBlocks()
{
	std::sort(
		m_FreeList.begin(), m_FreeList.end(), [](const Header& a, const Header& b)
		{
			return a.offset < b.offset;
		});

	std::vector<Header> merged;
	for (const auto& block : m_FreeList)
	{
		if (!merged.empty() && merged.back().offset + merged.back().size == block.offset)
		{
			merged.back().size += block.size;
		}
		else
		{
			merged.push_back(block);
		}
	}

	m_FreeList = std::move(merged);
}