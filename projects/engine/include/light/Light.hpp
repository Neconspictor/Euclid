#pragma once
#include <glm/glm.hpp>
#include <util/Projectional.hpp>

class DirectionalLight : public Projectional
{
public:
	explicit DirectionalLight();
	virtual ~DirectionalLight();

	const glm::vec3& getColor() const;


	void setColor(glm::vec3 color);

protected:
	glm::vec3 color;
};

class PointLight : public Projectional
{
public:
	PointLight();
	virtual ~PointLight();

	const glm::mat4* getMatrices() const;

	float getRange() const;

protected:
	glm::mat4 shadowMatrices[6];

	void update() override;
};