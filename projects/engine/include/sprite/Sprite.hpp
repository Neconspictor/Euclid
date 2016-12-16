#pragma once
#include <glm/detail/type_vec2.hpp>
#include <model/Model.hpp>
#include <texture/Texture.hpp>

/**
 * A sprite is a 2D image texture that is drawn onto the screen.
 * This class serves as an interface for renderer independent Sprite drawing.
 */
class Sprite
{
public:
	Sprite(int width, int height, glm::vec2 position);
	virtual ~Sprite();

	int getHeight() const;
	glm::vec2 getPosition() const;
	int getWidth() const;

	void setHeight(int height);
	void setPosition(glm::vec2 position);
	void setWidth(int width);

	virtual void draw() = 0;

	void setTexture(Texture* texture);

protected:
	int width;
	int height;
	glm::vec2 screenPosition;
	Texture* texture;
};