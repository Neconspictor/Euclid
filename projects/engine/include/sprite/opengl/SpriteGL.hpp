#pragma once
#include <sprite/Sprite.hpp>
#include <texture/opengl/TextureGL.hpp>

class SpriteGL : Sprite
{
public:
	SpriteGL(int width, int height, const glm::vec2& position, TextureGL* texture);

	virtual ~SpriteGL();

	Texture* getTexture() const;

	void draw() override;

private:
	TextureGL* texture;
};