#pragma once

// Class to store all data related to a Raster Tile
class RasterTileData
{
public:
	RasterTileData();
	RasterTileData(unsigned char* image, int height, int width, int zoom, int x, int y);
	~RasterTileData();
public:
	int m_zoom;
	int m_x, m_y;
	int m_height, m_width;
	unsigned char* m_image;
};