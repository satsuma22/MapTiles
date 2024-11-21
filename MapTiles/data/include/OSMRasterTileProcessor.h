#pragma once

#include "OSMRasterTileLoader.h"

// Class to process the raw response from the OSMRasterTileLoader class
class OSMRasterTileProcessor
{
public:
	OSMRasterTileProcessor(OSMRasterTileLoader& loader);

	inline unsigned char* GetImage() const { return m_image; }
	inline int GetHeight() const { return m_height; }
	inline int GetWidth() const { return m_width; }
	inline int GetSize() const { return m_size; }

private:
	unsigned char* m_image;
	int m_height, m_width, m_nchannels, m_size;
};