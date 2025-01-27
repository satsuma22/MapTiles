#pragma once

#include <GL/glew.h>

#include "opengl/VertexArray.h"
#include "opengl/Shader.h"

#include "opengl/RasterTileRender.h"
#include "opengl/Tile3DRender.h"
#include "opengl/Attribution.h"


class Renderer
{
public:
	void Draw(const VertexBuffer& vb, const Shader& shader) const ;
	void Draw(const Attribution& attribution, const Shader& shader) const;
	void Draw(const RasterTileRender& tile, const Shader& shader) const;
	void Draw(const Tile3DRender& tile, const Shader& shader) const;
	void Clear() const;
private:

};