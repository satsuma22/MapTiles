#pragma once

#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/VertexBufferLayout.h"

#include "Tile3DData.h"

#include <memory>

// Class to store opengl/rendering related information of a Tile3D
class Tile3DRender
{
public:
	Tile3DRender() {};
	Tile3DRender(const Tile3DData& tile, double refLat, double refLon);
	~Tile3DRender();
public:
	std::shared_ptr<VertexArray> m_vertexArray;
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
};