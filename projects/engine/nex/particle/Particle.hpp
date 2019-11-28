#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace nex {
	class Particle {
	public:

		static constexpr float GRAVITY = -9.81f;

		Particle(const glm::vec3& pos, 
			const glm::vec3& vel, 
			float roation, 
			float scale, 
			float lifeTime, 
			float gravityInfluence);

		float getElapsedTime() const;
		float getGravityInfluence() const;
		float getLifeTime() const;
		const glm::vec3& getPosition() const;
		float getRotation() const;
		float getScale() const;
		const glm::vec3& getVelocity() const;

		bool isAlive() const;

		void setGravityInfluence(float influence);
		void setLifeTime(float lifeTime);
		void setPosition(const glm::vec3& pos);
		void setRotation(float rotation);
		void setScale(float scale);
		void setVelocity(const glm::vec3& vel);

		/**
		 * Updates the particle for the current frame. 
		 * Return value indicates if the particle is still alive.
		 */
		bool update(const glm::mat4& view, float frameTime);

	private:

		glm::vec3 mPosition;
		glm::vec3 mVelocity;
		float mRotation;
		float mScale;
		float mGravityInfluence;
		float mLifeTime;
		float mElapsedTime;
		bool mIsAlive;

		//TODO: not for every particle! Do it in the shader
		glm::mat4 mModelView;
	};
}