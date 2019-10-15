#include <nex/texture/Sprite.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/EffectLibrary.hpp>
#include <nex/effects/SpritePass.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>

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

void nex::Sprite::render(SpritePass* spritePass)
{
	thread_local auto* renderBackend = RenderBackend::get();
	static auto state = RenderState::createNoDepthTest();

	if (spritePass == nullptr) {
		auto* lib = renderBackend->getEffectLibrary();
		spritePass = lib->getSpritePass();
	}

	spritePass->bind();
	spritePass->setTexture(mTexture);
	spritePass->setTransform(mTransform);

	renderBackend->setViewPort(mScreenPosition.x, mScreenPosition.y, mWidth, mHeight);
	StaticMeshDrawer::drawFullscreenTriangle(state, spritePass);
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