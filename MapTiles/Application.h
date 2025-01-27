#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#include "opengl/Camera.h"
#include "opengl/Renderer.h"
#include "opengl/Shader.h"

#include "Config.h"
#include "TileManager.h"
#include "WGS84toCartesian.hpp"

class Application
{
public:
	Application(double lat, double lon, GlobalConfig config);
	int Init();
	void Run();
	void Terminate();

private:
	void ProcessApplicationInput();

private:
	GlobalConfig m_Config;
	TileManager m_TileManager;
	Renderer m_Renderer;
	Camera m_Camera;
	GLFWwindow* m_Window;
};