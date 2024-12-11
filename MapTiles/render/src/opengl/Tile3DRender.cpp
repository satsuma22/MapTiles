#include "opengl/Tile3DRender.h"

#include <glm.hpp>

#include <vector>


Tile3DRender::Tile3DRender(const Tile3DData& tile, double refLat, double refLon)
	: m_VertexArray(nullptr), m_VertexBuffer(nullptr)
{
	m_VertexArray = std::make_shared<VertexArray>();

	m_VertexBuffer = std::make_shared<VertexBuffer>(tile.m_meshCartesian.data(), tile.m_meshCartesian.size() * sizeof(glm::vec3));

	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(3);
	layout.Push<float>(3);

	m_VertexArray->AddBuffer(*m_VertexBuffer, layout);
}

Tile3DRender::~Tile3DRender()
{

}
