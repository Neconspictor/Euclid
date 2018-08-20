#pragma once
#include <glm/detail/type_vec2.hpp>
#include <nex/texture/Texture.hpp>
#include <glm/detail/type_vec3.hpp>

/**
 * A sprite is a 2D image texture that is drawn onto the screen.
 * This class serves as an interface for renderer independent Sprite drawing.
 */
class Sprite
{
public:
	Sprite();
	virtual ~Sprite();

	float getHeight() const;
	glm::vec2 getPosition() const;
	const glm::vec3& getRotation() const;
	Texture* getTexture() const;
	float getWidth() const;

	void setHeight(float height);
	void setPosition(glm::vec2 position);
	void setXRotation(float value);
	void setYRotation(float value);
	void setZRotation(float value);
	void setTexture(Texture* texture);
	void setWidth(float width);

protected:
	float relativeHeight;
	float relativeWidth;
	glm::vec3 rotation;
	glm::vec2 screenPosition;
	Texture* texture;
};