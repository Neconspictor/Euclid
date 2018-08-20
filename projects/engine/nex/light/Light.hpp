#pragma once
#include <glm/glm.hpp>
#include <nex/util/Projectional.hpp>

class DirectionalLight : public Projectional
{
public:
	explicit DirectionalLight();
	virtual ~DirectionalLight();

	const glm::vec3& getColor() const;

	const glm::vec3& getDirection() const;

	void setColor(glm::vec3 color);

	void setDirection(glm::vec3 dir);

protected:
	glm::vec3 color;
	glm::vec3 direction;
};

class PointLight : public Projectional
{
public:
	PointLight();
	virtual ~PointLight();

	glm::mat4* getMatrices();

	float getRange() const;

	void setRange(float range);

protected:
	glm::mat4 shadowMatrices[6];

	void update() override;
};