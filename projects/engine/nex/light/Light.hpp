#pragma once
#include <glm/glm.hpp>
#include <nex/util/Projectional.hpp>

namespace nex {

	class DirectionalLight : public Projectional
	{
	public:
		explicit DirectionalLight();

		const glm::vec3& getColor() const;

		const glm::vec3& getDirection() const;

		float getLightPower() const;

		void setColor(glm::vec3 color);

		void setDirection(glm::vec3 dir);

		void setPower(float power);

	protected:
		glm::vec3 mColor;
		glm::vec3 mDirection;
		float mPower;
	};

	class PointLight : public Projectional
	{
	public:
		PointLight();

		glm::mat4* getMatrices();

		float getRange() const;

		void setRange(float range);

	protected:
		glm::mat4 shadowMatrices[6];

		void update(bool alwaysUpdate = false) override;
	};

}