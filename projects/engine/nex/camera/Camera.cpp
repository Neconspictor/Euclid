#include <nex/camera/Camera.hpp>
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.inl>

//using namespace std;
//using namespace glm;
//using namespace platform;

Camera::Camera() : Projectional()
{
	m_logger.setPrefix("Camera");
}

Camera::Camera(glm::vec3 position, glm::vec3 look, glm::vec3 up) : Camera()
{
	this->position = std::move(position);
	this->look = std::move(look);
	this->up = std::move(up);
}

Camera::Camera(const Camera& other) :  Projectional(other)
{
}

Camera::~Camera()
{
}

void Camera::update(Input* input, float frameTime)
{
	double yOffset = input->getFrameScrollOffsetY();
	
	// zoom
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= (float)yOffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;

	//revalidate = true;
}