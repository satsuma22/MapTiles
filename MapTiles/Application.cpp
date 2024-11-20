#include "Application.h"

#include <iostream>
#include <stb_image.h>
#include <Windows.h>

#include "opengl/Attribution.h"

void ProcessInput(GLFWwindow* window, Camera& camera, float delta)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.MoveCamera(Camera::FORWARD, delta);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.MoveCamera(Camera::BACKWARD, delta);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.MoveCamera(Camera::LEFT, delta);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.MoveCamera(Camera::RIGHT, delta);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.MoveCamera(Camera::LEFT_SPIN, delta);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.MoveCamera(Camera::RIGHT_SPIN, delta);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.MoveCamera(Camera::UP_SPIN, delta);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.MoveCamera(Camera::DOWN_SPIN, delta);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


Application::Application(double lat, double lon, GlobalConfig config)
{
    m_Window = nullptr;
	m_Config = config;
    
    Shader::ShaderDirectories.push_back("./");
    Shader::ShaderDirectories.push_back("../MapTiles/render/glsl/");
    Shader::ShaderDirectories.push_back("../../MapTiles/render/glsl/");
    Shader::ShaderDirectories.push_back("render/glsl/");

    std::array<double, 2> pos = wgs84::toCartesian({ m_Config.ReferencePoint.lat, m_Config.ReferencePoint.lon }, { lat, lon });
    float altitude = 100;

    m_Camera = Camera(glm::vec3(pos[0], 100, -pos[1]), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    m_TileManager.Init(lat, lon, altitude, m_Config);
}

int Application::Init()
{

    // Initialize the library 
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context 
    m_Window = glfwCreateWindow(800, 600, "Research Project", NULL, NULL);
    if (!m_Window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current 
    glfwMakeContextCurrent(m_Window);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW not initialized.\n";
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    return 1;
}

void Application::Run()
{
    // Set up the shaders
    Shader rasterTileShader("RasterTileVertex.glsl", "RasterTileFragment.glsl");
    rasterTileShader.SetUniform1i("u_Texture", 0);

    Shader tile3DShader("Tile3DVertex.glsl", "Tile3DFragment.glsl");

    //================================ Attribution =======================================================
    Shader attributionShader("AttributionVertex.glsl", "AttributionFragment.glsl");
    Attribution attribution("attribution.png");
    //====================================================================================================

    //m_TileManager.Update();

    glEnable(GL_DEPTH_TEST);

    double deltaTime = 0;
    double lastTime = glfwGetTime();
    int fps = m_Config.max_fps;

    while (!glfwWindowShouldClose(m_Window))
    {
        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (fps && deltaTime < 1.0 / fps)
        {
            Sleep((1.0 / fps - deltaTime) * 1000);
        }
        //std::cout << "(OPENGL) FPS: " << 1 / deltaTime << "\n";

        glm::vec3 cameraPos = m_Camera.GetProjectedPosition();
        std::array<double, 2> cameraPosWGS84 = wgs84::fromCartesian({m_Config.ReferencePoint.lat, m_Config.ReferencePoint.lon}, {cameraPos.x, cameraPos.y});

        m_TileManager.SetPosition(cameraPosWGS84[0], cameraPosWGS84[1], cameraPos.z);
        m_TileManager.Update();

        ProcessInput(m_Window, m_Camera, deltaTime);

        glm::mat4 VP(1.0f);
        VP = m_Camera.GetProjectionMatrix() * m_Camera.GetViewMatrix();
        rasterTileShader.SetUniformMat4f("ViewProjectionMatrix", VP);

        // Render here 
        m_Renderer.Clear();
        glClearColor(0.18, 0.27, 0.51, 1.0f);
        //glClearColor(0.4, 0.4, 0.5, 1.0f);

        for (auto& rasterTile : m_TileManager.m_ActiveRasterTiles)
        {
            m_Renderer.Draw(rasterTile.second, rasterTileShader);
        }

        cameraPos = m_Camera.GetPosition();
        tile3DShader.SetUniformMat4f("ViewProjectionMatrix", VP);
        tile3DShader.SetUniform3f("CameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

        for (auto& tile3D : m_TileManager.m_ActiveTile3Ds)
        {
            m_Renderer.Draw(tile3D.second, tile3DShader);
        }

        m_Renderer.Draw(attribution, attributionShader);

        // Swap front and back buffers 
        glfwSwapBuffers(m_Window);

        // Poll for and process events 
        glfwPollEvents();
    }
}

void Application::Terminate()
{
    m_TileManager.Finalize();
    glfwTerminate();
}
