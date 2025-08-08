#include "VertexBuffer.h"

VertexBuffer::VertexBuffer()
{
	m_FreeList.push_back({ 0,m_TotalSize });
	glGenBuffers(1, &m_RendererId);
	Bind();
	glBufferData(
		GL_ARRAY_BUFFER, 
		m_TotalSize, 
		nullptr, 
		GL_DYNAMIC_DRAW
	);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_RendererId);
}

void VertexBuffer::Bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
};

void VertexBuffer::UnBind() const {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int VertexBuffer::Allocate(const void* data, const size_t& size)
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

			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
			return static_cast<int>(offset);
		}
		
	}

	size_t oldSize = m_TotalSize;
	m_TotalSize *= 2;

	unsigned int temp;
	glGenBuffers(1, &temp);
	glBindBuffer(GL_ARRAY_BUFFER, temp);
	
	glBufferData(
		GL_ARRAY_BUFFER,
		m_TotalSize,
		nullptr,
		GL_DYNAMIC_DRAW
	);

	glBindBuffer(GL_COPY_READ_BUFFER, m_RendererId);
	glBindBuffer(GL_COPY_WRITE_BUFFER, temp);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, oldSize);

	glDeleteBuffers(1, &m_RendererId);
	m_RendererId = temp;

	m_FreeList.push_back({ oldSize, oldSize });
	MergeFreeBlocks();

	Bind();

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	return Allocate(data, size);
}

void VertexBuffer::Free(const size_t& offset, const size_t& size)
{
	m_FreeList.push_back({ offset, size });
	MergeFreeBlocks();
}

void VertexBuffer::MergeFreeBlocks()
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