#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace nex
{
	/**
	 * Defines a space transformation that transforms from local space to a specified space.
	 **/
	struct SpaceTrafo {
	public:

		const glm::vec4& getPerspective() const;
		const glm::vec3& getPosition() const;
		const glm::quat& getRotation() const;
		const glm::vec3& getScale() const;
		const glm::vec3& getShear() const;
		const glm::mat4& getTrafo() const;

		void setPerspective(const glm::vec4& vec);
		void setPosition(const glm::vec3& vec);
		void setRotation(const glm::quat& q);
		void setScale(const glm::vec3& vec);
		void setShear(const glm::vec3& vec);
		void setTrafo(const glm::mat4& mat);

		void update();

	private:

		void compose();
		void decompose();

		glm::vec3 calcScaleFromTrafo() const;

		glm::vec3 mPosition = glm::vec3(0.0f);
		glm::quat mRotation = glm::quat();
		glm::vec3 mScale = glm::vec3(1.0f);
		glm::vec3 mShear = glm::vec3(0.0f);
		glm::vec4 mPerspective = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::mat4 mTrafo = glm::mat4(1.0f);
		bool mNeedsCompose = false;
		bool mNeedsDecompose = false;
	};


	/**
	 * Transforms between to transformation spaces
	 */
	struct TransformationSpace
	{
	public:

	private:

		SpaceTrafo mLocalToSource;
		SpaceTrafo mLocalToTarget;
	};
}