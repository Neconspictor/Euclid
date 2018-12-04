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

float Sprite::getHeight() const
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

float Sprite::getWidth() const
{
	return relativeWidth;
}

void Sprite::setHeight(float height)
{
	this->relativeHeight = height;
}

void Sprite::setPosition(vec2 position)
{
	screenPosition = move(position);
}

void Sprite::setXRotation(float value)
{
	this->rotation.x = value;
}

void Sprite::setYRotation(float value)
{
	rotation.y = value;
}

void Sprite::setZRotation(float value)
{
	rotation.z = value;
}

void Sprite::setWidth(float width)
{
	this->relativeWidth = width;
}

void Sprite::setTexture(Texture* texture)
{
	this->texture = texture;
}