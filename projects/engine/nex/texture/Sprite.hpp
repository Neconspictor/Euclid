#pragma once
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
	class RenderBackend;
	class SpritePass;

	/**
	 * A sprite is a 2D image texture that is drawn onto the screen.
	 * This class serves as an interface for renderer independent Sprite drawing.
	 */
	class Sprite
	{
	public:
		Sprite();
		virtual ~Sprite();

		unsigned getHeight() const;
		glm::uvec2 getPosition() const;
		Texture* getTexture() const;
		unsigned getWidth() const;

		/**
		 * Renders the sprite into the current active render target.
		 */
		void render(SpritePass* pass);

		/**
		 * Sets the position of the sprite in screen space.
		 * Note: The origin of the sprite is the lower left corner.
		 */
		void setPosition(const glm::uvec2& position);
		void setTransform(const glm::mat4& mat);
		void setTexture(Texture* texture);
		void setWidth(unsigned width);
		void setHeight(unsigned height);

	protected:
		unsigned mWidth;
		unsigned mHeight;
		glm::uvec2 mScreenPosition;
		glm::mat4 mTransform;
		Texture* mTexture;
	};
}