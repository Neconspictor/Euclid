#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/light/Light.hpp>

//#include <glm/gtc/matrix_transform.inl>

using namespace std;
//using namespace glm;

namespace nex {

	DirectionalLight::DirectionalLight() : Projectional(), mColor(1, 1, 1), mDirection(1,1,1), mPower(1.0f)
	{
		m_logger.setPrefix("DirectionalLight");
	}

	const glm::vec3& DirectionalLight::getColor() const
	{
		return mColor;
	}

	const glm::vec3& DirectionalLight::getDirection() const
	{
		return mDirection;
	}

	float DirectionalLight::getLightPower() const
	{
		return mPower;
	}

	void DirectionalLight::setPower(float power)
	{
		mPower = power;
	}

	void DirectionalLight::setColor(glm::vec3 color)
	{
		this->mColor = move(color);
	}

	void DirectionalLight::setDirection(glm::vec3 dir)
	{
		mDirection = move(dir);
	}

	PointLight::PointLight() : Projectional()
	{
		fov = 90.0f;
		aspectRatio = 1.0f;
		perspFrustum.nearPlane = 0.1f;
		perspFrustum.farPlane = 1.0f;
		m_logger.setPrefix("PointLight");
	}

	glm::mat4* PointLight::getMatrices()
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

	void PointLight::update(bool alwaysUpdate)
	{
		if (revalidate || alwaysUpdate)
		{
			glm::vec3 test2 = position;
			glm::vec3 test = glm::vec3(1.0, 0.0, 0.0);
			test2 + test;

			glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspectRatio, perspFrustum.nearPlane, perspFrustum.farPlane);
			// right plane
			shadowMatrices[0] = shadowProj * glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0),
				glm::vec3(0.0, -1.0, 0.0));
			// left plane
			shadowMatrices[1] = shadowProj * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0),
				glm::vec3(0.0, -1.0, 0.0));
			// top plane
			shadowMatrices[2] = shadowProj * glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0),
				glm::vec3(0.0, 0.0, 1.0));
			// bottom plane
			shadowMatrices[3] = shadowProj * glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0),
				glm::vec3(0.0, 0.0, -1.0));
			// front plane
			shadowMatrices[4] = shadowProj * glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0),
				glm::vec3(0.0, -1.0, 0.0));
			// back plane
			shadowMatrices[5] = shadowProj * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0),
				glm::vec3(0.0, -1.0, 0.0));
		}
		Projectional::update();
	}

}