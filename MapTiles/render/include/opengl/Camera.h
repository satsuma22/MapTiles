#include <glm.hpp>

class Camera
{
public:
	enum MovementType
	{
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN,
        UP_SPIN,
        DOWN_SPIN,
        LEFT_SPIN,
        RIGHT_SPIN,
	};

public:
    Camera() {}
    Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 worldup);

    inline const glm::mat4& GetViewMatrix() { return m_View; }
    inline const glm::mat4& GetProjectionMatrix() { return m_Projection; }

    glm::vec3 GetPosition() { return m_Position; }
    glm::vec3 GetProjectedPosition();

    void MoveCamera(MovementType type, float delta);

private:
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();
    void Update();

private:
    glm::mat4 m_View;
    glm::mat4 m_Projection;

    glm::vec3 m_Position;
    glm::vec3 m_Forward;
    glm::vec3 m_Right;
    glm::vec3 m_Up;
    glm::vec3 m_WorldUp;

    float m_Speed;
    float m_RotationSpeed;
    float m_FOV;
    float m_Height;
    float m_Width;
    float m_NearClip;
    float m_FarClip;
};