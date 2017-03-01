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
	fov = 90.0f;
	aspectRatio = 1.0f;
	perspFrustum.nearPlane = 0.1f;
	perspFrustum.farPlane = 1.0f;
	logClient.setPrefix("[PointLight]");
}

PointLight::~PointLight()
{
}

mat4* PointLight::getMatrices()
{
	update();
	return shadowMatrices;
}

float PointLight::getRange() const
{
	return perspFrustum.farPlane;
}

void PointLight::setRange(float range)
{
	perspFrustum.farPlane = range;
	revalidate = true;
}

void PointLight::update()
{
	if (revalidate)
	{
		mat4 shadowProj = glm::perspective(radians(90.0f), aspectRatio, perspFrustum.nearPlane, perspFrustum.farPlane);
		// right plane
		shadowMatrices[0] = shadowProj * glm::lookAt(position, position + vec3(1.0, 0.0, 0.0),
			vec3(0.0, -1.0, 0.0));
		// left plane
		shadowMatrices[1] = shadowProj * glm::lookAt(position, position + vec3(-1.0, 0.0, 0.0),
			vec3(0.0, -1.0, 0.0));
		// top plane
		shadowMatrices[2] = shadowProj * glm::lookAt(position, position + vec3(0.0, 1.0, 0.0),
			vec3(0.0, 0.0, 1.0));
		// bottom plane
		shadowMatrices[3] = shadowProj * glm::lookAt(position, position + vec3(0.0, -1.0, 0.0),
			vec3(0.0, 0.0, -1.0));
		// front plane
		shadowMatrices[4] = shadowProj * glm::lookAt(position, position + vec3(0.0, 0.0, 1.0),
			vec3(0.0, -1.0, 0.0));
		// back plane
		shadowMatrices[5] = shadowProj * glm::lookAt(position, position + vec3(0.0, 0.0, -1.0),
			vec3(0.0, -1.0, 0.0));
	}
	Projectional::update();
}
