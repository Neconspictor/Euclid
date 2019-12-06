#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nex/common/FrameUpdateable.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/renderer/RenderCommand.hpp>
#include <functional>
#include <nex/material/Material.hpp>
#include <nex/mesh/MeshGroup.hpp>


namespace nex {

	class MeshGroup;
	struct RenderCommand;
	class RenderCommandQueue;
	class ParticleManager;
	class Particle;
	class ParticleSystem;
	class Texture;

	using ParticleIterator = std::vector<Particle>::const_iterator;

	struct ParticleRange {
		ParticleIterator begin;
		ParticleIterator end;
	};

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

	class ParticleShader : public Shader {
	public:

		class Material : public nex::Material {
		public:

			Material(Shader* shader);
			virtual ~Material() = default;

			Texture* texture = nullptr;
			glm::vec4 color;
		};

		ParticleShader();
		virtual ~ParticleShader() = default;

		void setLifeTimePercentage(float percentage);
		
		void updateConstants(const Constants& constants) override;
		void updateInstance(const glm::mat4& modelMatrix, const glm::mat4& prevModelMatrix, const void* data = nullptr) override;
		void updateMaterial(const nex::Material& material) override;

		

	private:
		Uniform mViewProj;
		Uniform mInverseView3x3;
		Uniform mModel;
		Uniform mColor;
		Uniform mTileCount;
		Uniform mLifeTimePercentage;
		UniformTex mTexture;
	};


	class ParticleRenderer {
	public:
		ParticleRenderer(const Material* material);

		void createRenderCommands(
			const ParticleIterator& begin,
			const ParticleIterator& end,
			const nex::AABB* boundingBox,
			RenderCommandQueue& commandQueue);

		static RenderState createParticleRenderState();
		static void createParticleMaterial(Material* material);

		/**
		 * Note: the data property of the command parameter needs to point to a valid ParticleRange.
		 */
		static void render(const RenderCommand& command,
			Shader** lastShaderPtr,
			const Constants& constants,
			const ShaderOverride<nex::Shader>& overrides,
			const RenderState* overwriteState);

		~ParticleRenderer();

	private:
		MeshBatch mParticleMB;
		RenderCommand mPrototype;
		ParticleRange mRange;
	};


	class ParticleManager {
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

		void frameUpdate(const glm::mat4& view, float frameTime);
		
		ParticleIterator getParticleBegin() const;
		ParticleIterator getParticleEnd() const;		

	private:

		std::vector<Particle> mParticles;
		int mLastActive;
	};

	class ParticleSystem  : public FrameUpdateable {
	public:
		ParticleSystem(
			const AABB& boundingBox,
			float gravityInfluence,
			float lifeTime,
			std::unique_ptr<Material> material,
			size_t maxParticles,
			const glm::vec3& position,
			float pps, 
			float rotation,
			float scale,
			float speed);

		virtual ~ParticleSystem() = default;


		void collectRenderCommands(RenderCommandQueue& commandQueue);

		void frameUpdate(const Constants& constants) override;

		const nex::AABB& getBoundingBox() const;
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
		nex::AABB mBox;
		std::unique_ptr<Material> mMaterial;

		ParticleManager mManager;
		ParticleRenderer mRenderer;
		
		void emit(const glm::vec3& center);
	};

	class VarianceParticleSystem : public FrameUpdateable {
	public:

		VarianceParticleSystem(
			float averageLifeTime,
			float averageScale,
			float averageSpeed,
			const AABB& boundingBox,
			float gravityInfluence,
			std::unique_ptr<Material> material,
			size_t maxParticles,
			const glm::vec3& position,
			float pps,
			float rotation,
			bool randomizeRotation);

		virtual ~VarianceParticleSystem() = default;

		void collectRenderCommands(RenderCommandQueue& commandQueue);

		void frameUpdate(const Constants& constants) override;

		const nex::AABB& getBoundingBox() const;
		const glm::vec3& getPosition() const;

		void setDirection(const glm::vec3& direction, float directionDeviation);

		/**
		 * @param variance : in range [0, 1]
		 */
		void setLifeVariance(float variance);

		void setPosition(const glm::vec3& pos);

		/**
		 * @param variance : in range [0, 1]
		 */
		void setScaleVariance(float variance);

		/**
		 * @param variance : in range [0, 1]
		 */
		void setSpeedVariance(float variance);

	protected:
		float mAverageLifeTime;
		float mAverageScale;
		float mAverageSpeed;
		nex::AABB mBox;
		float mGravityInfluence;
		ParticleManager mManager;
		std::unique_ptr<Material> mMaterial;
		float mPartialParticles;
		glm::vec3 mPosition;
		float mPps;
		float mRotation;
		bool mRandomizeRotation;
		ParticleRenderer mRenderer;

		float mSpeedVariance = 0, mLifeVariance = 0, 
			mScaleVariance = 0;
		glm::vec3 mDirection;
		float mDirectionDeviation = 0;
		bool mUseCone;

		void emit(const glm::vec3& center);

		static glm::vec3 generateRandomUnitVector();
		static glm::vec3 generateRandomUnitVectorWithinCone(const glm::vec3& dir, float angle);
		float generateRotation() const;
		static float generateValue(float average, float variance);
	};
}