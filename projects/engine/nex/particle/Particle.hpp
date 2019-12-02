#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nex/common/FrameUpdateable.hpp>
#include <nex/math/BoundingBox.hpp>

namespace nex {

	class MeshGroup;
	struct RenderCommand;
	class RenderCommandQueue;
	class ParticleManager;
	class Particle;

	using ParticleIterator = std::vector<Particle>::const_iterator;

	class Particle {
	public:

		Particle(const glm::vec3& pos = glm::vec3(0.0),
			const glm::vec3& vel = glm::vec3(0.0),
			float rotation = 0.0f,
			float scale = 1.0f,
			float lifeTime = 0.0f,
			float gravityInfluence = 1.0f);


		static constexpr float GRAVITY = -9.81f;

		const nex::AABB& getBoundingBox() const;

		float getElapsedTime() const;
		float getGravityInfluence() const;
		float getLifeTime() const;
		const glm::vec3& getPosition() const;
		float getRotation() const;
		float getScale() const;
		const glm::vec3& getVelocity() const;

		const glm::mat4& getWorldTrafo() const;

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

		friend ParticleManager;

		glm::vec3 mPosition;
		glm::vec3 mVelocity;
		float mRotation;
		float mScale;
		float mGravityInfluence;
		float mLifeTime;
		float mElapsedTime;
		bool mIsAlive;

		//TODO: not for every particle! Do it in the shader
		glm::mat4 mWorldTrafo;
		nex::AABB mBox;
	};

	class ParticleRenderer {
	public:
		ParticleRenderer();

		void createRenderCommands(ParticleIterator& begin, ParticleIterator& end, RenderCommandQueue& commandQueue);

		~ParticleRenderer();

	private:
		class ParticleShader;
		std::unique_ptr<ParticleShader> mShader;
		std::unique_ptr<MeshGroup> mParticleMG;
		std::vector<RenderCommand> mPrototypes;
	};


	class ParticleManager : public FrameUpdateable {
	public:
		ParticleManager(size_t maxParticles);

		/**
		 * Creates a new particle
		 */
		void create(const glm::vec3& pos,
			const glm::vec3& vel,
			float rotation,
			float scale,
			float lifeTime,
			float gravityInfluence);

		void createRenderCommands(RenderCommandQueue& commandQueue);

		void frameUpdate(const Constants& constants) override;

		ParticleIterator getParticleBegin() const;
		ParticleIterator getParticleEnd() const;

	private:
		std::vector<Particle> mParticles;
		int mLastActive;
		ParticleRenderer mRenderer;
	};
}