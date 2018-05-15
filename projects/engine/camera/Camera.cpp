#include <camera/Camera.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;
using namespace platform;

Camera::Camera() : Projectional()
{
	logClient.setPrefix("[Camera]");
}

Camera::Camera(vec3 position, vec3 look, vec3 up) : Camera()
{
	this->position = move(position);
	this->look = move(look);
	this->up = move(up);
}

Camera::Camera(const Camera& other) :  Projectional(other)
{
}

Camera::~Camera()
{
}

void Camera::update(Input* input, float frameTime)
{
}


void Camera::onScroll(double xOffset, double yOffset)
{
	// zoom
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= (float)yOffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;

	revalidate = true;
}