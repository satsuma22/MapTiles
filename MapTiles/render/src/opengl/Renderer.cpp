#include "opengl/Renderer.h"

void Renderer::Draw(const VertexBuffer& vb, const Shader& shader) const
{
	shader.Bind();
	vb.Bind();

	glDrawArrays(GL_TRIANGLES, 0, vb.GetSize());
}

void Renderer::Draw(const Attribution& attribution, const Shader& shader) const
{
	attribution.m_VertexArray->Bind();
	attribution.m_Texture->Bind();

	Draw(*(attribution.m_VertexBuffer), shader);
}

void Renderer::Draw(const RasterTileRender& tile, const Shader& shader) const
{
	tile.m_VertexArray->Bind();
	tile.m_Texture->Bind();
	
	Draw(*(tile.m_VertexBuffer), shader);
}

void Renderer::Draw(const Tile3DRender& tile, const Shader& shader) const
{
	tile.m_VertexArray->Bind();
	Draw(*(tile.m_VertexBuffer), shader);
}

void Renderer::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
