#include <light/Light.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;

DirectionalLight::DirectionalLight() : Projectional(), color(1,1,1)
{
}

DirectionalLight::~DirectionalLight()
{
}

const glm::vec3& DirectionalLight::getColor() const
{
	return color;
}

void DirectionalLight::setColor(glm::vec3 color)
{
	this->color = move(color);
}
