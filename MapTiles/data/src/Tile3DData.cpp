#include "Tile3DData.h"

#include "WGS84toCartesian.hpp"

Tile3DData::Tile3DData()
	: m_latMin(0), m_lonMin(0), m_latMax(0), m_lonMax(0)
{

}

Tile3DData::Tile3DData(double latMin, double lonMin, double latMax, double lonMax, std::vector<glm::dvec3>& mesh)
	: m_latMin(latMin), m_lonMin(lonMin), m_latMax(latMax), m_lonMax(lonMax)
{
	m_meshWGS = std::move(mesh);

	// add a rect to every tile that covers the entire tile
	m_meshWGS.push_back({ m_latMin, m_lonMin, -50 });
	m_meshWGS.push_back({ .8, .8, .8 });
	m_meshWGS.push_back({ m_latMax, m_lonMin, -50 });
	m_meshWGS.push_back({ .8, .8, .8 });
	m_meshWGS.push_back({ m_latMax, m_lonMax, -50 });
	m_meshWGS.push_back({ .8, .8, .8 });
	m_meshWGS.push_back({ m_latMin, m_lonMin, -50 });
	m_meshWGS.push_back({ .8, .8, .8 });
	m_meshWGS.push_back({ m_latMax, m_lonMax, -50 });
	m_meshWGS.push_back({ .8, .8, .8 });
	m_meshWGS.push_back({ m_latMin, m_lonMax, -50 });
	m_meshWGS.push_back({ .8, .8, .8 });
}

void Tile3DData::ConvertTo3DCoordinates(double refLat, double refLon)
{
	
	for (int i = 0; i < m_meshWGS.size(); i += 6)
	{
		auto p0 = wgs84::toCartesian({ refLat, refLon }, { m_meshWGS[i].x, m_meshWGS[i].y });
		auto p1 = wgs84::toCartesian({ refLat, refLon }, { m_meshWGS[i + 2].x, m_meshWGS[i + 2].y });
		auto p2 = wgs84::toCartesian({ refLat, refLon }, { m_meshWGS[i + 4].x, m_meshWGS[i + 4].y });

		glm::vec3 v0 = { p0[0], m_meshWGS[i][2]    , -p0[1]};
		glm::vec3 v1 = { p1[0], m_meshWGS[i + 2][2], -p1[1]};
		glm::vec3 v2 = { p2[0], m_meshWGS[i + 4][2], -p2[1]};


		glm::vec3 normal = glm::normalize(glm::cross((v0 - v1), (v2 - v1)));

		m_meshCartesian.push_back(v0);
		m_meshCartesian.push_back(m_meshWGS[i + 1]);
		m_meshCartesian.push_back(normal);
		m_meshCartesian.push_back(v1);
		m_meshCartesian.push_back(m_meshWGS[i + 3]);
		m_meshCartesian.push_back(normal);
		m_meshCartesian.push_back(v2);
		m_meshCartesian.push_back(m_meshWGS[i + 5]);
		m_meshCartesian.push_back(normal);
	}
}
