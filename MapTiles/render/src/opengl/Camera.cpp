#include "opengl/Camera.h"

#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>

#include <algorithm>

Camera::Camera()
    :   m_View(0), m_Projection(0), m_Position(0), m_Forward(0), m_Right(0), m_Up(0), m_WorldUp(0),
        m_Speed(0), m_RotationSpeed(0), m_FOV(0), m_Height(0), m_Width(0), m_NearClip(0), m_FarClip(0)
{
}

Camera::Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 worldup)
    : m_View(1.0f), m_Projection(1.0f), m_Right(0.0f), m_Up(0.0f)
{
	m_Position = pos;
	m_Forward = dir;
	m_WorldUp = worldup;

    m_Speed = 100.0f;
    m_RotationSpeed = 2.0f;
    m_FOV = 75.0f;
    m_Height = 1080;
    m_Width = 1920;
    m_NearClip = 0.1f;
    m_FarClip = 5000.0f;

    Update();
    UpdateProjectionMatrix();
}

glm::vec3 Camera::GetProjectedPosition() const
{
    float t = -m_Position.z / (std::min(-0.25f, m_Forward.z));
    glm::vec3 ProjectedPos = m_Position + t * m_Forward;
    ProjectedPos.z = m_Position.z;

    return ProjectedPos;
}

void Camera::MoveCamera(MovementType type, float delta)
{
    float fCameraSpeed = m_Speed * delta;
    float fCameraRotationSpeed = m_RotationSpeed * delta;

    float pitchDelta = 0;
    float yawDelta = 0;

    switch (type)
    {
    case FORWARD:
    {
        m_Position += m_Forward * fCameraSpeed;
        break;
    }
    case BACKWARD:
    {
        m_Position -= m_Forward * fCameraSpeed;
        break;
    }
    case LEFT:
    {
        m_Position -= m_Right * fCameraSpeed;
        break;
    }
    case RIGHT:
    {
        m_Position += m_Right * fCameraSpeed;
        break;
    }
    case UP:
    {
        m_Position += m_WorldUp * fCameraSpeed;
        break;
    }
    case DOWN:
    {
        m_Position -= m_WorldUp * fCameraSpeed;
        break;
    }
    case LEFT_SPIN:
        yawDelta -= fCameraRotationSpeed;
        break;

    case RIGHT_SPIN:
        yawDelta += fCameraRotationSpeed;
        break;

    case UP_SPIN:
        pitchDelta += fCameraRotationSpeed;
        break;

    case DOWN_SPIN:
        pitchDelta -= fCameraRotationSpeed;
        break;
    }

    if (yawDelta != 0 || pitchDelta != 0)
    {
        glm::quat q = glm::normalize(glm::cross(glm::angleAxis(pitchDelta, m_Right), glm::angleAxis(-yawDelta, m_WorldUp)));
        m_Forward = glm::rotate(q, m_Forward);
    }

    Update();
}

void Camera::UpdateViewMatrix()
{
    m_View = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
}

void Camera::UpdateProjectionMatrix()
{
    m_Projection = glm::perspective(glm::radians(m_FOV), (float)m_Width / (float)m_Height, m_NearClip, m_FarClip);
}

void Camera::Update()
{
    m_Right = glm::normalize(glm::cross(m_Forward, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Forward));

    UpdateViewMatrix();
}
