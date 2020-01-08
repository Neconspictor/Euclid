#include <nex/camera/FPCamera.hpp>
#include <nex/gui/ImGUI_Extension.hpp>
#include <imgui/imgui.h>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <nex/platform/Input.hpp>

using namespace std;
using namespace glm;

nex::FPCamera::FPCamera(float width, float height) : PerspectiveCamera(width, height), mYaw(0), mPitch(0)
{
	mLogger.setPrefix("FPCamera");
}

void nex::FPCamera::setLook(vec3 direction)
{
	// it is assumed that look is a normalized vector -> important for arcsinus!
	Camera::setLook(direction);

	mYaw = degrees(atan2(mCoordSystem.look.x, -mCoordSystem.look.z));

	// look.y = sin(radians(-pitch));
	// <-> -degrees(asin(look.y)) = pitch
	mPitch = degrees(asin(-mCoordSystem.look.y));
	mPitch = limit(mPitch, -89.0f, 89.0f);
}

void nex::FPCamera::frameUpdate(Input* input, float frameTime)
{
	float sensitivity = 0.05f;
	MouseOffset data = input->getFrameMouseOffset();
	float yawAddition = static_cast<float>(data.xOffset) * sensitivity;
	float pitchAddition = static_cast<float>(data.yOffset) * sensitivity;
	mYaw += yawAddition;
	mPitch += pitchAddition;

	mPitch = limit(mPitch, -89.0f, 89.0f);

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
	return mYaw;
}

float nex::FPCamera::getPitch() const
{
	return mPitch;
}

void nex::FPCamera::setYaw(float yaw)
{
	mYaw = yaw;
}

void nex::FPCamera::setPitch(float pitch)
{
	mPitch = pitch;
}

void nex::FPCamera::recalculateLookVector()
{
	vec3 front;
	front.x = sin(radians(mYaw)) * cos(radians(mPitch));
	front.y = sin(radians(-mPitch));
	front.z = -cos(radians(mYaw)) * cos(radians(mPitch));
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


nex::FPCamera_ConfigurationView::FPCamera_ConfigurationView(FPCamera* camera) : mCamera(camera)
{
	
}

void nex::FPCamera_ConfigurationView::drawSelf()
{
	// render configuration properties
	ImGui::PushID(mId.c_str());
	ImGui::DragFloat("yaw", &mCamera->mYaw, 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat("pitch", &mCamera->mPitch, 1.0f, -89.0f, 89.0f);


	float fovY = glm::degrees(mCamera->getFovY());
	ImGui::DragFloat("fov", &fovY, 1.0f, -1000.0f, 1000.0f);
	mCamera->setFovY(glm::radians(fovY));
	ImGui::DragFloat("aspect ratio", &mCamera->mAspectRatio, 0.1f, 0.1f, 90.0f);
	ImGui::DragFloat("near plane", &mCamera->mDistanceNear, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat("far plane", &mCamera->mDistanceFar, 1.0f, 1.0f, 10000.0f);
	ImGui::DragFloat("speed", &mCamera->mCameraSpeed, 0.2f, 0.0f, 100.0f);


	glm::vec3 position = mCamera->getPosition();
	if (nex::gui::Vector3D(&position, "Position")) {
		mCamera->setPosition(position, true);
	}


	mCamera->recalculateLookVector();
	ImGui::PopID();
}