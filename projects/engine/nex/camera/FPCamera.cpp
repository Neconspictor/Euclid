#include <nex/camera/FPCamera.hpp>
#include <nex/gui/Util.hpp>
#include <imgui/imgui.h>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

using namespace std;
using namespace glm;

nex::FPCamera::FPCamera() : FPCameraBase(), yaw(0), pitch(0)
{
	m_logger.setPrefix("FPCamera");
}

nex::FPCamera::FPCamera(vec3 position, vec3 look, vec3 up) : FPCameraBase(position, look, up),
	yaw(0), pitch(0)
{
	m_logger.setPrefix("FPCamera");
}

nex::FPCamera::FPCamera(const FPCamera& other) : FPCameraBase(other)
{
	yaw = other.yaw;
	pitch = other.pitch;
	m_logger.setPrefix("FPCamera");
}

float nex::FPCamera::limit(float source, float minValue, float maxValue)
{
	if (source > maxValue)
		source = maxValue;
	if (source < minValue)
		source = minValue;
	return source;
}

void nex::FPCamera::setLook(vec3 direction)
{
	// it is assumed that look is a normalized vector -> important for arcsinus!
	Camera::setLook(direction);

	yaw = degrees(atan2(look.x, -look.z));

	// look.y = sin(radians(-pitch));
	// <-> -degrees(asin(look.y)) = pitch
	pitch = degrees(asin(-look.y));
	pitch = limit(pitch, -89.0f, 89.0f);
}

void nex::FPCamera::update(Input* input, float frameTime)
{
	FPCameraBase::update(input, frameTime);
	float sensitivity = 0.05f;
	MouseOffset data = input->getFrameMouseOffset();
	float yawAddition = static_cast<float>(data.xOffset) * sensitivity;
	float pitchAddition = static_cast<float>(data.yOffset) * sensitivity;
	yaw += yawAddition;
	pitch += pitchAddition;

	pitch = limit(pitch, -89.0f, 89.0f);

	recalculateLookVector();
	
	//Camera::setLookDirection(direction);
	doUserMovement(input, frameTime);

	//smooth transition from current position to target position using lerping 
	// A damping factor slows down speed over time.
	static const float DAMPING = 1.0f;
	const float alpha = std::clamp<float>(frameTime*2.0*DAMPING, 0.0f, 1.0f);
	mCurrentPosition = glm::lerp<glm::vec3>(mCurrentPosition, mTargetPosition, glm::vec3(alpha));
}

float nex::FPCamera::getYaw() const
{
	return yaw;
}

float nex::FPCamera::getPitch() const
{
	return pitch;
}

void nex::FPCamera::setYaw(float yaw)
{
	this->yaw = yaw;
}

void nex::FPCamera::setPitch(float pitch)
{
	this->pitch = pitch;
}

void nex::FPCamera::recalculateLookVector()
{
	vec3 front;
	front.x = sin(radians(yaw)) * cos(radians(pitch));
	front.y = sin(radians(-pitch));
	front.z = -cos(radians(yaw)) * cos(radians(pitch));
	front = normalize(front);
	//look = normalize(front);
	setLook(front);
}


nex::FPCamera_ConfigurationView::FPCamera_ConfigurationView(FPCamera* camera) : m_camera(camera)
{
	
}

void nex::FPCamera_ConfigurationView::drawSelf()
{
	// render configuration properties
	ImGui::PushID(m_id.c_str());
	ImGui::DragFloat("yaw", &m_camera->yaw, 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat("pitch", &m_camera->pitch, 1.0f, -89.0f, 89.0f);
	ImGui::DragFloat("fov", &m_camera->fov, 1.0f, 0.0f, 90.0f);
	ImGui::DragFloat("aspect ratio", &m_camera->aspectRatio, 0.1f, 0.1f, 90.0f);
	ImGui::DragFloat("near plane", &m_camera->perspFrustum.nearPlane, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat("far plane", &m_camera->perspFrustum.farPlane, 1.0f, 1.0f, 10000.0f);
	ImGui::DragFloat("speed", &m_camera->cameraSpeed, 0.2f, 0.0f, 100.0f);

	nex::gui::Vector3D(&m_camera->mCurrentPosition, "Position");

	m_camera->recalculateLookVector();
	ImGui::PopID();
}