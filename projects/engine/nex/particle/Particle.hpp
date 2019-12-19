#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nex/common/FrameUpdateable.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/renderer/RenderCommand.hpp>
#include <functional>
#include <nex/material/Material.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/scene/Vob.hpp>


namespace nex {

	class MeshGroup;
	struct RenderCommand;
	class RenderCommandQueue;
	class ParticleManager;
	class Particle;
	class ParticleSystem;
	class Texture;
	class ShaderStorageBuffer;

	using ParticleIterator = std::vector<Particle>::const_iterator;

	struct ParticleRange {
		ParticleIterator begin;
		ParticleIterator end;
	};

	class Particle {
	public:

		Particle(const glm::vec3& pos = glm::vec3(0.0),
			const glm::vec3& vel = glm::vec3(0.0),
			const glm::vec3& dampedVel = glm::vec3(0.0),
			float rotation = 0.0f,
			float scale = 1.0f,
			float lifeTime = 0.0f,
			float gravityInfluence = 1.0f);


		static constexpr float GRAVITY = -9.81f;

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
		bool update(const glm::vec3& velocity, float frameTime);

		void updateWorldTrafo(const glm::mat4& invViewWithoutPosition);

	private:

		friend ParticleManager;

		glm::vec3 mPosition;
		glm::vec3 mTargetPosition;
		glm::vec3 mVelocity;
		
		float mRotation;
		float mScale;
		float mGravityInfluence;
		float mLifeTime;
		float mElapsedTime;
		bool mIsAlive;

		glm::mat4 mWorldTrafo;
	};

	class ParticleShader : public Shader {
	public:

		class Material : public nex::Material {
		public:

			Material(std::shared_ptr<ShaderProvider> provider);
			virtual ~Material() = default;

			Texture* texture = nullptr;
			glm::vec4 color;
			ShaderStorageBuffer* instanceBuffer = nullptr;
		};

		struct ParticleData {
			glm::mat4 worldTrafo; // has alignment of vec4
			float lifeTimePercentage;
			float _pad[3]; // for alignmnet reasons
		};

		ParticleShader();
		virtual ~ParticleShader() = default;

		void setLifeTimePercentage(float percentage);
		
		void updateConstants(const Constants& constants) override;
		void updateInstance(const glm::mat4& modelMatrix, const glm::mat4& prevModelMatrix, const void* data = nullptr) override;
		void updateMaterial(const nex::Material& material) override;

		void bindParticlesBuffer(ShaderStorageBuffer* buffer);

		

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
			size_t activeParticleCount,
			const nex::AABB* boundingBox,
			RenderCommandQueue& commandQueue,
			bool doCulling);

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
			const glm::vec3& dampedVel,
			float rotation,
			float scale,
			float lifeTime,
			float gravityInfluence);

		void frameUpdate(const glm::vec3& velocity, float frameTime);
		void updateParticleTrafos(const glm::mat4& invViewWithoutPosition);
		
		ParticleIterator getParticleBegin() const;
		ParticleIterator getParticleEnd() const;

		const Particle* getParticles() const;

		size_t getActiveParticleCount() const;
		size_t getBufferSize() const;

		const AABB& getBoundingBox() const;

		void sortActiveParticles(const glm::vec3& cameraPosition);

	private:

		std::vector<Particle> mParticles;
		int mLastActive;
		AABB mBoundingBox;
	};

	class VarianceParticleSystem : public Vob, public FrameUpdateable {
	public:

		VarianceParticleSystem(
			float averageLifeTime,
			float averageScale,
			float averageSpeed,
			const AABB& boundingBox,
			float gravityInfluence,
			std::unique_ptr<ParticleShader::Material> material,
			size_t maxParticles,
			const glm::vec3& position,
			float pps,
			float rotation,
			bool randomizeRotation,
			bool sortParticles);

		virtual ~VarianceParticleSystem() = default;

		void collectRenderCommands(RenderCommandQueue& queue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer) override;

		void frameUpdate(const Constants& constants) override;

		void setDirection(const glm::vec3& direction, float directionDeviation);

		/**
		 * @param variance : in range [0, 1]
		 */
		void setLifeVariance(float variance);

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
		float mGravityInfluence;
		ParticleManager mManager;
		std::unique_ptr<ParticleShader::Material> mMaterial;
		float mPartialParticles;
		float mPps;
		float mRotation;
		bool mRandomizeRotation;
		bool mSortParticles;
		ParticleRenderer mRenderer;
		std::unique_ptr<ShaderStorageBuffer> mInstanceBuffer;
		std::vector<ParticleShader::ParticleData> mShaderParticles;

		float mSpeedVariance = 0, mLifeVariance = 0, 
			mScaleVariance = 0;
		glm::vec3 mDirection;
		float mDirectionDeviation = 0;
		bool mUseCone;
		glm::vec3 mOldPosition;

		void emit(const glm::vec3& center, const glm::vec3& psVelocity, size_t count);

		static glm::vec3 generateRandomUnitVector();
		static glm::vec3 generateRandomUnitVectorWithinCone(const glm::vec3& dir, float angle);
		float generateRotation() const;
		static float generateValue(float average, float variance);

		void recalculateLocalBoundingBox() override;
		void recalculateBoundingBoxWorld() override;
	};
}