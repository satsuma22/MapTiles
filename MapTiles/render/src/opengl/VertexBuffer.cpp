#include "opengl/VertexBuffer.h"
#include "opengl/Renderer.h"


VertexBuffer::VertexBuffer(const void* data, unsigned int size)
	: m_Size(size)
{
	glGenBuffers(1, &m_Id);
	glBindBuffer(GL_ARRAY_BUFFER, m_Id);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_Id);
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_Id);
}

void VertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::ReplaceData(const void* data, unsigned int size)
{
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}
