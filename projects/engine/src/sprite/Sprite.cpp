#include <sprite/Sprite.hpp>

using namespace std;
using namespace glm;

Sprite::Sprite(int width, int height, vec2 position) : width(width), height(height), screenPosition(position)
{
}

Sprite::~Sprite()
{
}

int Sprite::getHeight() const
{
	return height;
}

vec2 Sprite::getPosition() const
{
	return screenPosition;
}

int Sprite::getWidth() const
{
	return width;
}

void Sprite::setHeight(int height)
{
	this->height = height;
}

void Sprite::setPosition(vec2 position)
{
	screenPosition = move(position);
}

void Sprite::setWidth(int width)
{
	this->width = width;
}

void Sprite::setTexture(Texture* texture)
{
	this->texture = texture;
}