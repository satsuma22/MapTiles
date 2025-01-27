#include "opengl/VertexBufferLayout.h"

template<typename T>
void VertexBufferLayout::Push(unsigned int count)
{
	//static_assert(false);
}

template<>
void VertexBufferLayout::Push<double>(unsigned int count)
{
	m_Elements.push_back({ GL_DOUBLE, count, GL_FALSE });
	m_Stride += VertexBufferElement::GetSizeOfType(GL_DOUBLE) * count;
}

template<>
void VertexBufferLayout::Push<float>(unsigned int count)
{
	m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
	m_Stride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
}

template<>
void VertexBufferLayout::Push<unsigned int>(unsigned int count)
{
	m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
	m_Stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
}