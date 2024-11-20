#include "opengl/Tile3DRender.h"

#include <glm.hpp>
#include "WGS84toCartesian.hpp"

#include "../../Timer.h"

#include <vector>


Tile3DRender::Tile3DRender(const Tile3DData& tile, double refLat, double refLon)
	: m_vertexArray(nullptr), m_vertexBuffer(nullptr)
{
	m_vertexArray = std::make_shared<VertexArray>();

	m_vertexBuffer = std::make_shared<VertexBuffer>(tile.m_meshCartesian.data(), tile.m_meshCartesian.size() * sizeof(glm::vec3));

	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(3);
	layout.Push<float>(3);

	m_vertexArray->AddBuffer(*m_vertexBuffer, layout);
}

Tile3DRender::~Tile3DRender()
{

}
