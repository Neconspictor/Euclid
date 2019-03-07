#pragma once
#include <nex/camera/Camera.hpp>
#include <nex/light/Light.hpp>
#include <nex/texture/Sprite.hpp>
#include "nex/shader/ShaderBuffer.hpp"
#include "nex/Renderer.hpp"
#include "nex/shader/Shader.hpp"
#include "nex/texture/RenderTarget.hpp"
#include "nex/pbr/PBR_Deferred.hpp"
#include "SceneNearFarComputeShader.hpp"

namespace nex
{
	class ComputeTest_Renderer : public Renderer
	{
	public:

		class ComputeTestShader : public nex::ComputeShader
		{
		public:
			static const unsigned width = 2048;
			static const unsigned height = 1024;
			static const unsigned partitionCount = 6;

			struct BoundsFloat
			{
				glm::vec4 minCoord;
				glm::vec4 maxCoord;
			};

			struct Partition
			{
				glm::vec3 scale; // These are given in texture coordinate [0, 1] space
				float intervalBegin;
				glm::vec3 bias;  // These are given in texture coordinate [0, 1] space
				float intervalEnd;
			};

			struct Constant
			{
				glm::vec4 mCameraNearFar;
				glm::vec4 mColor;
				glm::mat4 mCameraProj;
				glm::mat4 mCameraViewToLightProj;
				Partition partitions[partitionCount];
			};

			struct WriteOut
			{
				//int lock = 0; float _pad0[3];
				BoundsFloat results[partitionCount];
				
			};

			struct LockBuffer
			{
				int lock = 0; float _pad0[3];
			};

			void setConstants(float viewNearZ, float viewFarZ, const glm::mat4& projection, const glm::mat4& cameraViewToLightProjection);
			ComputeTestShader(unsigned width, unsigned height);

			void setDepthTexture(Texture* depth, InternFormat format);
			
			void reset(WriteOut* out);
			
			Guard<Texture> depth;
			Guard<ShaderStorageBuffer> uniformBuffer;
			Guard<ShaderStorageBuffer> storageBuffer;
			Guard<ShaderStorageBuffer> lockBuffer;


		};

		class ComputeClearColorShader : public nex::ComputeShader
		{
		public:
			ComputeClearColorShader(Texture* texture);
		};

		class SimpleBlinnPhong : public Shader
		{
		public:
			SimpleBlinnPhong();

			void onModelMatrixUpdate(const glm::mat4 & modelMatrix) override;

			void setLightDirection(const glm::vec3 lightDirectionWorld);

			void setView(const glm::mat4* view);
			void setProjection(const glm::mat4* projection);

			void setViewPositionWorld(const glm::vec3 viewPositionWorld);

		private:
			Uniform mTransformMatrix;
			Uniform mModelMatrix;
			Uniform mViewPositionWorld;
			Uniform mDirLightDirection;
			const glm::mat4* mView;
			const glm::mat4* mProjection;
		};

		class SimpleGeometryShader : public Shader
		{
		public:
			SimpleGeometryShader();

			void onModelMatrixUpdate(const glm::mat4 & modelMatrix) override;

			void setView(const glm::mat4* view);
			void setProjection(const glm::mat4* projection);

		private:
			Uniform mTransformMatrix;
			const glm::mat4* mView;
			const glm::mat4* mProjection;
		};

		class GBuffer : public RenderTarget
		{
		public:
			GBuffer(unsigned width, unsigned height);

			Texture* getDepth()const;

		private:
			Texture* mDepth;
		};



		typedef unsigned int uint;

		ComputeTest_Renderer(RenderBackend* renderer, Input* input);

		void init(int windowWidth, int windowHeight);
		void render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight) override;

		void renderNew(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight);
		void renderOld(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight);
		void updateRenderTargets(int width, int height);


	private:

		// Allow the UI mode classes accessing private members

		DirectionalLight globalLight;
		nex::Logger m_logger;
		Texture* panoramaSky;
		Texture* testTexture;

		std::unique_ptr<RenderTarget2D> renderTargetSingleSampled;
		Sprite screenSprite;;
		std::unique_ptr<ComputeTestShader> mComputeTest;
		std::unique_ptr<ComputeClearColorShader> mComputeClearColor;
		std::unique_ptr<SimpleBlinnPhong> mSimpleBlinnPhong;
		std::unique_ptr<SimpleGeometryShader> mSimpleGeometry;
		std::unique_ptr<GBuffer> mGBuffer;
		std::unique_ptr<SceneNearFarComputeShader> mSceneNearFarComputeShader;
		Input* mInput;
	};
}
