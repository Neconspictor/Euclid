#include <nex/texture/Sprite.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/effects/EffectLibrary.hpp>
#include <nex/effects/SpriteShader.hpp>
#include <nex/renderer/Drawer.hpp>

using namespace std;
using namespace glm;
using namespace nex;

Sprite::Sprite() : mHeight(1.0f), 
    mWidth(1.0f), mScreenPosition({0,0}), mTexture(nullptr)
{
}

Sprite::~Sprite()
{
}

unsigned Sprite::getHeight() const
{
	return mHeight;
}

uvec2 Sprite::getPosition() const
{
	return mScreenPosition;
}

Texture* Sprite::getTexture() const
{
	return mTexture;
}

unsigned Sprite::getWidth() const
{
	return mWidth;
}

void nex::Sprite::render(SpriteShader* spriteShader)
{
	thread_local auto* renderBackend = RenderBackend::get();
	const auto& state = RenderState::getNoDepthTest();

	if (spriteShader == nullptr) {
		auto* lib = renderBackend->getEffectLibrary();
		spriteShader = lib->getSpritePass();
	}

	spriteShader->bind();
	spriteShader->update(mTexture, mTransform);

	renderBackend->setViewPort(mScreenPosition.x, mScreenPosition.y, mWidth, mHeight);
	Drawer::drawFullscreenTriangle(state, spriteShader);
}

void Sprite::setPosition(const glm::uvec2& position)
{
	mScreenPosition = position;
}


void nex::Sprite::setTransform(const glm::mat4& mat)
{
	mTransform = mat;
}

void Sprite::setTexture(Texture* texture)
{
	mTexture = texture;
}

void nex::Sprite::setWidth(unsigned width)
{
	mWidth = width;
}

void nex::Sprite::setHeight(unsigned height)
{
	mHeight = height;
}