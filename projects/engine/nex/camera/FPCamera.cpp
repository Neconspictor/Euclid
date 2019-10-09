#include <nex/camera/FPCamera.hpp>
#include <nex/gui/Util.hpp>
#include <imgui/imgui.h>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <nex/Input.hpp>

using namespace std;
using namespace glm;

nex::FPCamera::FPCamera(float aspectRatio) : PerspectiveCamera(aspectRatio), yaw(0), pitch(0)
{
	mLogger.setPrefix("FPCamera");
}

void nex::FPCamera::setLook(vec3 direction)
{
	// it is assumed that look is a normalized vector -> important for arcsinus!
	Camera::setLook(direction);

	yaw = degrees(atan2(mCoordSystem.look.x, -mCoordSystem.look.z));

	// look.y = sin(radians(-pitch));
	// <-> -degrees(asin(look.y)) = pitch
	pitch = degrees(asin(-mCoordSystem.look.y));
	pitch = limit(pitch, -89.0f, 89.0f);
}

void nex::FPCamera::frameUpdate(Input* input, float frameTime)
{
	float sensitivity = 0.05f;
	MouseOffset data = input->getFrameMouseOffset();
	float yawAddition = static_cast<float>(data.xOffset) * sensitivity;
	float pitchAddition = static_cast<float>(data.yOffset) * sensitivity;
	yaw += yawAddition;
	pitch += pitchAddition;

	pitch = limit(pitch, -89.0f, 89.0f);

	recalculateLookVector();



	// camera movements
	const float moveAmount = mCameraSpeed * frameTime;

	vec3 direction(0.0f);

	if (input->isDown(Input::KEY_W))
		direction += mCoordSystem.look;

	if (input->isDown(Input::KEY_S))
		direction -= mCoordSystem.look;

	if (input->isDown(Input::KEY_D))
		direction += mRight;

	if (input->isDown(Input::KEY_A))
		direction -= mRight;

	if (input->isDown(Input::KEY_Z))
		direction += mCoordSystem.up;

	if (input->isDown(Input::KEY_X))
		direction -= mCoordSystem.up;

	if (length(direction) > 0)
		mTargetPosition += moveAmount * normalize(direction);

	
	PerspectiveCamera::frameUpdate(input, frameTime);
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

float nex::FPCamera::limit(float source, float minValue, float maxValue)
{
	if (source > maxValue)
		source = maxValue;
	if (source < minValue)
		source = minValue;
	return source;
}


nex::FPCamera_ConfigurationView::FPCamera_ConfigurationView(FPCamera* camera) : m_camera(camera)
{
	
}

void nex::FPCamera_ConfigurationView::drawSelf()
{
	// render configuration properties
	ImGui::PushID(mId.c_str());
	ImGui::DragFloat("yaw", &m_camera->yaw, 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat("pitch", &m_camera->pitch, 1.0f, -89.0f, 89.0f);


	float fovY = glm::degrees(m_camera->getFovY());
	ImGui::DragFloat("fov", &fovY, 1.0f, -1000.0f, 1000.0f);
	m_camera->setFovY(glm::radians(fovY));
	ImGui::DragFloat("aspect ratio", &m_camera->mAspectRatio, 0.1f, 0.1f, 90.0f);
	ImGui::DragFloat("near plane", &m_camera->mDistanceNear, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat("far plane", &m_camera->mDistanceFar, 1.0f, 1.0f, 10000.0f);
	ImGui::DragFloat("speed", &m_camera->mCameraSpeed, 0.2f, 0.0f, 100.0f);

	nex::gui::Vector3D(&m_camera->mCoordSystem.position, "Position");

	m_camera->recalculateLookVector();
	ImGui::PopID();
}