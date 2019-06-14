#pragma once
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
	/**
	 * A sprite is a 2D image texture that is drawn onto the screen.
	 * This class serves as an interface for renderer independent Sprite drawing.
	 */
	class Sprite
	{
	public:
		Sprite();
		virtual ~Sprite();

		Real getHeight() const;
		glm::vec2 getPosition() const;
		const glm::vec3& getRotation() const;
		Texture* getTexture() const;
		Real getWidth() const;

		void setHeight(Real height);
		void setPosition(glm::vec2 position);
		void setXRotation(Real value);
		void setYRotation(Real value);
		void setZRotation(Real value);
		void setTexture(Texture* texture);
		void setWidth(Real width);

		static const Sprite& getScreenSprite();

	protected:
		Real relativeHeight;
		Real relativeWidth;
		glm::vec3 rotation;
		glm::vec2 screenPosition;
		Texture* texture;
	};
}