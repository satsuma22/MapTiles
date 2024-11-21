#pragma once

#include <opengl/Texture.h>
#include <opengl/VertexArray.h>
#include <opengl/VertexBuffer.h>
#include <opengl/VertexBufferLayout.h>

#include <stb_image.h>

#include <memory>
#include <fstream>
#include <iostream>

class Attribution
{
public:
	Attribution(std::string filename) 
	{
		stbi_set_flip_vertically_on_load(1);
		
		m_AttributionDirectories.push_back("./");
		m_AttributionDirectories.push_back("../MapTiles/render/res/");
		m_AttributionDirectories.push_back("../../MapTiles/render/res/");
		m_AttributionDirectories.push_back("render/res/");

		std::string path;
		std::fstream file;

		for (auto& directory : m_AttributionDirectories)
		{
			file.open(directory + filename);
			if (file.is_open())
			{
				path = (directory + filename);
				break;
			}
		}

		if (!file.is_open())
			std::cout << "WARNING: Couldn't find attribution image.\n";
		else
			file.close();

		int width, height, nChannels;
		const char* filePath = path.c_str();

		unsigned char* image = stbi_load(filePath, &width, &height, &nChannels, 4);

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

	std::vector<std::string> m_AttributionDirectories;
};