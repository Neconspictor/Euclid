#include "fly_camera.h"

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

// --------------------------------------------------------------------------------------------
nex::FlyCamera::FlyCamera(glm::vec3 position, glm::vec3 forward, glm::vec3 up) : CellCamera(position, forward, up)
{
    Yaw = -90.0f;

    Forward = forward;
    m_WorldUp = Up;
    m_TargetPosition = position;
}
// --------------------------------------------------------------------------------------------
void nex::FlyCamera::Update(float dt)
{
	nex::CellCamera::Update(dt);
    // slowly interpolate to target position each frame given some damping factor.
    // this gives smooth camera movement that fades out the closer we are to our target.
    
	glm::vec3 clampValue(std::clamp<float>(dt * Damping * 2.0f, 0.0, 1.0));
	Position = glm::lerp(Position, m_TargetPosition, clampValue);

    Yaw      = glm::lerp(Yaw, m_TargetYaw, std::clamp<float>(dt * Damping * 2.0f, 0.0, 1.0));
    Pitch    = glm::lerp(Pitch, m_TargetPitch, std::clamp<float>(dt * Damping * 2.0f, 0.0, 1.0));

    // calculate new cartesian basis vectors from yaw/pitch pair:
    glm::vec3 newForward;
    newForward.x = cos(0.0174533 * Pitch) * cos(0.0174533 * Yaw);
    newForward.y = sin(0.0174533 * Pitch);
    newForward.z = cos(0.0174533 * Pitch) * sin(0.0174533 * Yaw);
    Forward = glm::normalize(newForward);
    Right = glm::normalize(glm::cross(Forward, m_WorldUp));
    Up = glm::cross(Right, Forward);

    // calculate the new view matrix
    UpdateView();
}
// --------------------------------------------------------------------------------------------
void nex::FlyCamera::InputKey(float dt, nex::CAMERA_MOVEMENT direction)
{
    float speed = MovementSpeed * dt;
    if (direction      == nex::CAMERA_FORWARD)
        m_TargetPosition = m_TargetPosition + Forward*speed;
    else if (direction == nex::CAMERA_BACK)
        m_TargetPosition = m_TargetPosition - Forward*speed;
    else if (direction == CAMERA_LEFT)
        m_TargetPosition = m_TargetPosition - Right*speed;
    else if (direction == CAMERA_RIGHT)
        m_TargetPosition = m_TargetPosition + Right*speed;
    else if (direction == CAMERA_UP)
        m_TargetPosition = m_TargetPosition + m_WorldUp*speed;
    else if (direction == CAMERA_DOWN)
        m_TargetPosition = m_TargetPosition - m_WorldUp*speed;
}
// --------------------------------------------------------------------------------------------
void nex::FlyCamera::InputMouse(float deltaX, float deltaY)
{
    float xmovement = deltaX * MouseSensitivty;
    float ymovement = deltaY * MouseSensitivty;

    m_TargetYaw   += xmovement;
    m_TargetPitch += ymovement;

    // prevents calculating the length of the null vector
    if(m_TargetYaw == 0.0f) m_TargetYaw = 0.01f;
    if(m_TargetPitch == 0.0f) m_TargetPitch = 0.01f;

    // it's not allowed to move the pitch above or below 90 degrees asctime the current 
    // world-up vector would break our LookAt calculation.
    if (m_TargetPitch > 89.0f)  m_TargetPitch =  89.0f;
    if (m_TargetPitch < -89.0f) m_TargetPitch = -89.0f;
}
// --------------------------------------------------------------------------------------------
void nex::FlyCamera::InputScroll(float deltaX, float deltaY)
{
    MovementSpeed = std::clamp<float>(MovementSpeed + deltaY * 1.0f, 1.0f, 25.0f); 
    Damping       = std::clamp<float>(Damping       + deltaX * 0.5f, 1.0f, 25.0f);
}