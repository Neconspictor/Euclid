#include "camera_frustum.h"

#include "camera.h"

nex::CameraFrustum::~CameraFrustum() = default;
// ------------------------------------------------------------------------
void nex::CameraFrustum::Update(CellCamera* camera)
{
    float tan = 2.0 * std::tan(camera->FOV * 0.5);
    float nearHeight = tan * camera->Near;
    float nearWidth  = nearHeight * camera->Aspect;
    float farHeight  = tan * camera->Far;
    float farWidth   = farHeight * camera->Aspect;

	glm::vec3 nearCenter = camera->Position + camera->Forward * camera->Near;
	glm::vec3 farCenter  = camera->Position + camera->Forward * camera->Far;

	glm::vec3 v;
    // left plane
    v = (nearCenter - camera->Right * nearWidth * 0.5f) - camera->Position;
    Left.SetNormalD(glm::cross(glm::normalize(v), camera->Up), nearCenter - camera->Right * nearWidth * 0.5f);
    // right plane
    v = (nearCenter + camera->Right * nearWidth  * 0.5f) - camera->Position;
    Right.SetNormalD(glm::cross(camera->Up, glm::normalize(v)), nearCenter + camera->Right * nearWidth * 0.5f);
    // top plane
    v = (nearCenter + camera->Up * nearHeight * 0.5f) - camera->Position;
    Top.SetNormalD(glm::cross(glm::normalize(v), camera->Right), nearCenter + camera->Up * nearHeight * 0.5f);
    // bottom plane
    v = (nearCenter - camera->Up * nearHeight * 0.5f) - camera->Position;
    Bottom.SetNormalD(glm::cross(camera->Right, glm::normalize(v)), nearCenter - camera->Up * nearHeight * 0.5f);
    // near plane
    Near.SetNormalD(camera->Forward, nearCenter);
    // far plane
    Far.SetNormalD(-camera->Forward, farCenter);
}
// ------------------------------------------------------------------------
bool nex::CameraFrustum::Intersect(glm::vec3 point)
{
    for (int i = 0; i < 6; ++i)
    {
        if (Planes[i].Distance(point) < 0)
        {
            return false;
        }
    }
    return true;
}
// ------------------------------------------------------------------------
bool nex::CameraFrustum::Intersect(glm::vec3 point, float radius)
{
    for (int i = 0; i < 6; ++i)
    {
        if (Planes[i].Distance(point) < -radius)
        {
            return false;
        }
    }
    return true;
}
// ------------------------------------------------------------------------
bool nex::CameraFrustum::Intersect(glm::vec3 boxMin, glm::vec3 boxMax)
{
    for (int i = 0; i < 6; ++i)
    {
		glm::vec3 positive = boxMin;
        if (Planes[i].Normal.x >= 0)
        {
            positive.x = boxMax.x;
        }
        if (Planes[i].Normal.y >= 0)
        {
            positive.y = boxMax.y;
        }
        if (Planes[i].Normal.z >= 0)
        {
            positive.z = boxMax.z;
        }
        if(Planes[i].Distance(positive) < 0)
        {
            return false;
        }
    }
    return true;
}