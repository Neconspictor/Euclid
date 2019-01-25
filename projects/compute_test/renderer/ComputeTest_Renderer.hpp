#pragma once
#include <nex/camera/Camera.hpp>
#include <nex/light/Light.hpp>
#include <nex/texture/Sprite.hpp>
#include <nex/opengl/renderer/Renderer.hpp>
#include "nex/shader/ShaderBuffer.hpp"

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

		ComputeTest_Renderer(RendererOpenGL* renderer, Input* input);

		bool getShowDepthMap() const;
		void init(int windowWidth, int windowHeight);
		void render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight) override;
		void setShowDepthMap(bool showDepthMap);
		void updateRenderTargets(int width, int height);
		nex::HBAO_GL* getHBAO();

		PBR_DeferredGL* getPBR();


	private:

		Texture * renderAO(Camera* camera, Texture* gPosition, Texture* gNormal);

		void drawSceneToCascade(SceneNode* scene);

		// Allow the UI mode classes accessing private members

		GaussianBlurGL* blurEffect;
		DirectionalLight globalLight;
		nex::Logger m_logger;
		Texture* panoramaSky;
		Texture* testTexture;

		RenderTarget2D* renderTargetSingleSampled;
		Sprite screenSprite;
		bool showDepthMap;
		Guard<ComputeTestShader> mComputeTest;
		Guard<ComputeClearColorShader> mComputeClearColor;
		std::unique_ptr<SimpleBlinnPhong> mSimpleBlinnPhong;
		std::unique_ptr<SimpleGeometryShader> mSimpleGeometry;
		std::unique_ptr<GBuffer> mGBuffer;
		Input* mInput;
	};
}
