#include "RasterTileData.h"
#include "utils.h"

#include <iostream>

/*
RasterTileData::RasterTileData()
	: m_zoom(0), m_x(0), m_y(0), m_height(0), m_width(0), m_image(nullptr)
{
}
*/

RasterTileData::RasterTileData(unsigned char* image, int height, int width, int zoom, int x, int y)
	: m_zoom(zoom), m_x(x), m_y(y), m_height(height), m_width(width), m_image(image)
{
}

RasterTileData::RasterTileData(RasterTileData&& other) noexcept
{
	m_zoom = other.m_zoom;
	m_x = other.m_x;
	m_y = other.m_y;
	m_height = other.m_height;
	m_width = other.m_width;
	m_image = other.m_image;
	other.m_image = nullptr;
}

RasterTileData::~RasterTileData()
{
	std::cout << "RasterTileData Destructor\n";
	if (m_image != nullptr)
	{
		delete[] m_image;
		m_image = nullptr;
	}
}
