#include "Application.h"

#include <iostream>
#include <stb_image.h>

// Define a macro that includes the correct sleep function for the current platform
#ifdef _WIN32
    #include <Windows.h>
    #define Sleep(x) Sleep(x)
#else
    #include <unistd.h>
    #define Sleep(x) usleep(x * 1000)
#endif

#include "opengl/Attribution.h"
#include "Timer.h"

static int F_key_state = GLFW_RELEASE;
static int R_key_state = GLFW_RELEASE;

static void ProcessCameraInput(GLFWwindow* window, Camera& camera, float delta)
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

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
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

    m_Camera = Camera(glm::vec3(pos[0], 0, -pos[1]), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    m_TileManager.Init(lat, lon, altitude, &m_Config);
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
    m_Window = glfwCreateWindow(1080, 720, "MapTiles", NULL, NULL);
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
    int fps = m_Config.MaxFPS;

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

        //glm::vec3 cameraPos = m_Camera.GetProjectedPosition();
        glm::vec3 cameraPos = m_Camera.GetPosition();
        std::array<double, 2> cameraPosWGS84 = wgs84::fromCartesian({m_Config.ReferencePoint.lat, m_Config.ReferencePoint.lon}, {cameraPos.x, -cameraPos.z});
        glm::mat4 VP(1.0f);
        VP = m_Camera.GetProjectionMatrix() * m_Camera.GetViewMatrix();
        m_TileManager.CalculateViewFrustum(VP);
        m_TileManager.SetPosition(cameraPosWGS84[0], cameraPosWGS84[1], cameraPos.y);
        m_TileManager.Update();

        ProcessCameraInput(m_Window, m_Camera, deltaTime);
        ProcessApplicationInput();

        rasterTileShader.SetUniformMat4f("ViewProjectionMatrix", VP);

        // Render here 
        m_Renderer.Clear();
        glClearColor(0.18, 0.27, 0.51, 1.0f);
        //glClearColor(0.4, 0.4, 0.5, 1.0f);

        auto& activeRasterTiles = m_TileManager.GetActiveRasterTiles();
        for (auto& rasterTile : activeRasterTiles)
        {
            m_Renderer.Draw(rasterTile.second, rasterTileShader);
        }

        cameraPos = m_Camera.GetPosition();
        tile3DShader.SetUniformMat4f("ViewProjectionMatrix", VP);
        tile3DShader.SetUniform3f("CameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

        auto& activeTile3Ds = m_TileManager.GetActiveTile3Ds();
        for (auto& tile3D : activeTile3Ds)
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

void Application::ProcessApplicationInput()
{
	if (glfwGetKey(m_Window, GLFW_KEY_F) == GLFW_PRESS && F_key_state == GLFW_RELEASE)
	{
		F_key_state = GLFW_PRESS;
		m_Config.FrustumBasedTileGeneration = !m_Config.FrustumBasedTileGeneration;
		std::cout << "Switching to " << (m_Config.FrustumBasedTileGeneration ? "Frustum" : "Grid") << " based tile generation.\n";
	}
    else if (glfwGetKey(m_Window, GLFW_KEY_F) == GLFW_RELEASE && F_key_state == GLFW_PRESS)
    {
        F_key_state = GLFW_RELEASE;
    }

    if (glfwGetKey(m_Window, GLFW_KEY_R) == GLFW_PRESS && R_key_state == GLFW_RELEASE)
    {
        R_key_state = GLFW_PRESS;
		m_TileManager.ClearRenderCache();
    }
    else if (glfwGetKey(m_Window, GLFW_KEY_R) == GLFW_RELEASE && R_key_state == GLFW_PRESS)
    {
        R_key_state = GLFW_RELEASE;
    }
}
