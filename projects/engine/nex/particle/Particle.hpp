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

		ParticleManager();

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

		static ParticleManager* get();
		
		ParticleIterator getParticleBegin() const;
		ParticleIterator getParticleEnd() const;

		void init(size_t maxParticles);

		

	private:
		std::vector<Particle> mParticles;
		int mLastActive;
		ParticleRenderer mRenderer;
	};

	class ParticleSystem  : public FrameUpdateable {
	public:
		ParticleSystem(
			float gravityInfluence,
			float lifeTime,
			const glm::vec3& position,
			float pps, 
			float rotation,
			float scale,
			float speed);
		virtual ~ParticleSystem() = default;

		void frameUpdate(const Constants& constants) override;

		const glm::vec3& getPosition() const;
		void setPosition(const glm::vec3& pos);

	protected:
	
		float mGravityInfluence;
		float mLifeTime;
		glm::vec3 mPosition;
		float mPps;
		float mRotation;
		float mScale;
		float mSpeed;
		float mPartialParticles;
		
		void emit(const glm::vec3& center);
	};
}