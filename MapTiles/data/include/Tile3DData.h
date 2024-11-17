#pragma once

#include <glm.hpp>
#include <vector>

// Class to store all data related to a Tile3D
class Tile3DData
{
public:
	Tile3DData();
	Tile3DData(double latMin, double lonMin, double latMax, double lonMax, std::vector<glm::dvec3>& mesh);
	void ConvertTo3DCoordinates(double refLat, double refLon);
public:
	double m_latMin, m_lonMin, m_latMax, m_lonMax;
	std::vector<glm::dvec3> m_mesh;
};