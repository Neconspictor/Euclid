#include <nex/texture/Sprite.hpp>

using namespace std;
using namespace glm;
using namespace nex;

Sprite::Sprite() : relativeHeight(1.0f), 
    relativeWidth(1.0f), screenPosition({0,0}), texture(nullptr)
{
}

Sprite::~Sprite()
{
}

Real Sprite::getHeight() const
{
	return relativeHeight;
}

vec2 Sprite::getPosition() const
{
	return screenPosition;
}

const vec3& Sprite::getRotation() const
{
	return rotation;
}

Texture* Sprite::getTexture() const
{
	return texture;
}

Real Sprite::getWidth() const
{
	return relativeWidth;
}

void Sprite::setHeight(Real height)
{
	this->relativeHeight = height;
}

void Sprite::setPosition(vec2 position)
{
	screenPosition = move(position);
}

void Sprite::setXRotation(Real value)
{
	this->rotation.x = value;
}

void Sprite::setYRotation(Real value)
{
	rotation.y = value;
}

void Sprite::setZRotation(Real value)
{
	rotation.z = value;
}

void Sprite::setWidth(Real width)
{
	this->relativeWidth = width;
}

const Sprite& Sprite::getScreenSprite()
{
	static Sprite screenSprite;
	return screenSprite;
}

void Sprite::setTexture(Texture* texture)
{
	this->texture = texture;
}
