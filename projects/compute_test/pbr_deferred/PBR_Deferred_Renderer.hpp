#pragma once
#include <nex/event/Task.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/opengl/scene/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/texture/Sprite.hpp>
#include <nex/opengl/shading_model/PBR_DeferredGL.hpp>
#include <nex/gui/ControllerStateMachine.hpp>
#include <nex/opengl/renderer/Renderer.hpp>
#include "nex/opengl/post_processing/AmbientOcclusion.hpp"
#include "nex/opengl/shader/ShaderBufferGL.hpp"

namespace nex
{
	class PBR_Deferred_Renderer : public Renderer
	{
	public:

		


		class ComputeTestShader : public nex::ComputeShader
		{
		public:
			static const unsigned width = 2048;
			static const unsigned height = 1024;


			struct ShaderBuffer
			{
				glm::vec4 mCameraNearFar;
				glm::vec4 mColor;
				glm::mat4 mCameraProj;
				glm::mat4 mCameraViewToLightProj;
			};

			struct WriteOut
			{
				int lock = 0;
				float _pad3[3] = {-1000,-1000,-1000 };
				glm::vec3 minResult = glm::vec3(FLT_MAX);
				float _pad0 = -1000;
				glm::vec3 maxResult= glm::vec3(-FLT_MAX);
				float _pad1 = -1000;
			};
			
			ComputeTestShader(unsigned width, unsigned height);
			Guard<Texture> depth;
			Guard<ShaderStorageBufferGL> uniformBuffer;
			Guard<ShaderStorageBufferGL> storageBuffer;
		};

		class ComputeClearColorShader : public nex::ComputeShader
		{
		public:
			ComputeClearColorShader(Texture* texture);
		};



		typedef unsigned int uint;

		PBR_Deferred_Renderer(RendererOpenGL* renderer);

		bool getShowDepthMap() const;
		void init(int windowWidth, int windowHeight);
		void render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight) override;
		void setShowDepthMap(bool showDepthMap);
		void updateRenderTargets(int width, int height);
		nex::HBAO_GL* getHBAO();
		AmbientOcclusionSelector* getAOSelector();

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

		std::unique_ptr<PBR_DeferredGL> m_pbr_deferred;
		std::unique_ptr<PBR_GBuffer>  pbr_mrt;
		std::unique_ptr<CascadedShadowGL> m_cascadedShadow;

		AmbientOcclusionSelector m_aoSelector;

		RenderTarget* renderTargetSingleSampled;
		Sprite screenSprite;
		DepthMap* shadowMap;
		bool showDepthMap;
		Guard<ComputeTestShader> mComputeTest;
		Guard<ComputeClearColorShader> mComputeClearColor;
	};

	class PBR_Deferred_Renderer_ConfigurationView : public nex::gui::Drawable
	{
	public:
		PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer);

	protected:
		void drawSelf() override;

		PBR_Deferred_Renderer* m_renderer;
	};
}
