#include <light/Light.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

DirectionalLight::DirectionalLight() : Projectional(), color(1,1,1)
{
	logClient.setPrefix("[DirectionalLight]");
}

DirectionalLight::~DirectionalLight()
{
}

const vec3& DirectionalLight::getColor() const
{
	return color;
}

void DirectionalLight::setColor(vec3 color)
{
	this->color = move(color);
}

PointLight::PointLight() : Projectional()
{
	logClient.setPrefix("[PointLight]");
}

PointLight::~PointLight()
{
}

const mat4* PointLight::getMatrices() const
{
	return shadowMatrices;
}

float PointLight::getRange() const
{
	return frustum.farPlane;
}

void PointLight::update()
{
	if (revalidate)
	{
		// right plane
		shadowMatrices[0] = glm::lookAt(position, position + vec3(1.0, 0.0, 0.0), 
			vec3(0.0, 1.0, 0.0));
		// left plane
		shadowMatrices[1] = glm::lookAt(position, position + vec3(-1.0, 0.0, 0.0),
			vec3(0.0, 1.0, 0.0));
		// top plane
		shadowMatrices[1] = glm::lookAt(position, position + vec3(0.0, 1.0, 0.0),
			vec3(0.0, 0.0, 1.0));
		// bottom plane
		shadowMatrices[1] = glm::lookAt(position, position + vec3(0.0, -1.0, 0.0),
			vec3(0.0, 0.0, 1.0));
		// front plane
		shadowMatrices[1] = glm::lookAt(position, position + vec3(0.0, 0.0, 1.0),
			vec3(0.0, 1.0, 0.0));
		// back plane
		shadowMatrices[1] = glm::lookAt(position, position + vec3(0.0, 0.0, -1.0),
			vec3(0.0, 1.0, 0.0));
	}
	Projectional::update();
}
