#pragma once

#include <opengl/Texture.h>
#include <opengl/VertexArray.h>
#include <opengl/VertexBuffer.h>
#include <opengl/VertexBufferLayout.h>

#include <stb_image.h>

#include <memory>

class Attribution
{
public:
	Attribution(const char* filepath) 
	{
		stbi_set_flip_vertically_on_load(1);
		
		int width, height, nChannels;
		unsigned char* image = stbi_load(filepath, &width, &height, &nChannels, 4);

		m_Texture = std::make_shared<Texture>(image, height, width);

		float vertices[] = {
			0.2, -1.0 , 0, 0, 0,
			1.0, -1.0 , 0, 1, 0,
			1.0, -0.96, 0, 1, 1,
			0.2, -1.0 , 0, 0, 0,
			1.0, -0.96, 0, 1, 1,
			0.2, -0.96, 0, 0, 1
		};

		m_VertexBuffer = std::make_shared<VertexBuffer>(vertices, sizeof(vertices));
		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(2);

		m_VertexArray = std::make_shared<VertexArray>();
		m_VertexArray->AddBuffer(*m_VertexBuffer, layout);

		stbi_image_free(image);
	}

public:
	std::shared_ptr<VertexArray> m_VertexArray;
	std::shared_ptr<VertexBuffer> m_VertexBuffer;
	std::shared_ptr<Texture> m_Texture;
};