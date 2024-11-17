#include "opengl/Tile3DRender.h"

#include <glm.hpp>
#include "WGS84toCartesian.hpp"

#include "../../Timer.h"

#include <vector>


Tile3DRender::Tile3DRender(const Tile3DData& tile, double refLat, double refLon)
	: m_vertexArray(nullptr), m_vertexBuffer(nullptr)
{
	m_vertexArray = std::make_shared<VertexArray>();

	std::vector<glm::vec3> mesh;

	// We don't need to convert to 3d coordinates here if we use already did that using Tile3DData::ConvertTo3DCoordinates() in the TileManagerData
	// The benefit of doing that is we only need to convert coordinate once, the first time the tile is downloaded from the internet
	// If the coordinate conversion is done here, it needs to happen every time the tile comes inside the neighbourhood 
	
	//mesh.resize(tile.m_mesh.size());
	
	//for (int i = 0; i < tile.m_mesh.size(); i++)
		//mesh.push_back(tile.m_mesh[i]);

	for (int i = 0; i < tile.m_mesh.size(); i += 6)
	{
		
		auto p0 = wgs84::toCartesian({ refLat, refLon }, { tile.m_mesh[i].x, tile.m_mesh[i].y });
		auto p1 = wgs84::toCartesian({ refLat, refLon }, { tile.m_mesh[i + 2].x, tile.m_mesh[i + 2].y });
		auto p2 = wgs84::toCartesian({ refLat, refLon }, { tile.m_mesh[i + 4].x, tile.m_mesh[i + 4].y });

		glm::vec3 v0 = { p0[0], p0[1], tile.m_mesh[i][2] };
		glm::vec3 v1 = { p1[0], p1[1], tile.m_mesh[i + 2][2] };
		glm::vec3 v2 = { p2[0], p2[1], tile.m_mesh[i + 4][2] };


		glm::vec3 normal = glm::normalize(glm::cross((v0 - v1), (v2 - v1)));

		mesh.push_back(v0);
		mesh.push_back(tile.m_mesh[i+1]);
		mesh.push_back(normal);
		mesh.push_back(v1);
		mesh.push_back(tile.m_mesh[i+3]);
		mesh.push_back(normal);
		mesh.push_back(v2);
		mesh.push_back(tile.m_mesh[i + 5]);
		mesh.push_back(normal);
	}
	

	m_vertexBuffer = std::make_shared<VertexBuffer>(mesh.data(), mesh.size() * sizeof(glm::vec3));

	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(3);
	layout.Push<float>(3);

	m_vertexArray->AddBuffer(*m_vertexBuffer, layout);
}

Tile3DRender::~Tile3DRender()
{

}
