#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

	void ReplaceData(const void* data, unsigned int size);

	inline unsigned int GetSize() const { return m_Size; }

private:
	unsigned int m_Id;
	unsigned int m_Size;
};