#pragma once

#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/VertexBufferLayout.h"

#include "RasterTileData.h"

#include <memory>

// Class to store opengl/rendering related information of a Raster Tile
class RasterTileRender
{
public:
	RasterTileRender() {};
	RasterTileRender(const RasterTileData& tile, double refLat, double refLon);
	~RasterTileRender();

public:
	std::shared_ptr<Texture> m_texture;
	std::shared_ptr<VertexArray> m_vertexArray;
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
};