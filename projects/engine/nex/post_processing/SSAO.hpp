#pragma once

#include <vector>
#include <nex/gui/Drawable.hpp>
#include <glm/glm.hpp>

namespace nex
{

	class Texture2D;
	class RenderTarget;
	class RenderTarget2D;
	class Shader;
	class StaticMeshDrawer;
	class Pass;

	const int SSAO_SAMPLING_SIZE = 32;

	struct SSAOData {
		Real   bias;
		Real   intensity;
		Real   radius;
		Real   _pad0;

		glm::vec4 invFullResolution;

		glm::mat4 projection_GPass;
		glm::mat4 inv_projection_GPass;

		glm::vec4 samples[SSAO_SAMPLING_SIZE]; // the w component is not used (just for padding)!
	};


	class SSAO_Deferred {
	public:

		SSAO_Deferred(unsigned int windowWidth,
			unsigned int windowHeight);

		virtual ~SSAO_Deferred() = default;

		Texture2D* getAO_Result();
		Texture2D* getBlurredResult();

		Texture2D* getNoiseTexture();
		void onSizeChange(unsigned int newWidth, unsigned int newHeight);

		void renderAO(Texture* depth, const glm::mat4& projectionGPass);
		void blur();

		void displayAOTexture(Texture* aoTexture);

		SSAOData* getSSAOData();

		void setBias(Real bias);
		void setItensity(Real itensity);
		void setRadius(Real radius);


	private:

		static std::unique_ptr<RenderTarget2D> createSSAO_FBO(unsigned int width, unsigned int height);

		Real randomFloat(Real a, Real b);
		Real lerp(Real a, Real b, Real f);

		std::unique_ptr<Texture2D> noiseTexture;
		std::unique_ptr<RenderTarget2D> aoRenderTarget;
		std::unique_ptr<RenderTarget2D> tiledBlurRenderTarget;
		std::unique_ptr<nex::Pass> aoPass;
		std::unique_ptr<nex::Pass> tiledBlurPass;
		std::unique_ptr<nex::Pass> aoDisplayPass;

		unsigned int windowWidth;
		unsigned int windowHeight;
		unsigned int noiseTileWidth = 4;
		std::array<glm::vec3, SSAO_SAMPLING_SIZE> ssaoKernel;
		std::vector<glm::vec3> noiseTextureValues;

		SSAOData   m_shaderData;
	};

	class SSAO_ConfigurationView : public nex::gui::Drawable {
	public:
		SSAO_ConfigurationView(SSAO_Deferred* ssao);

	protected:
		void drawSelf() override;

	private:
		SSAO_Deferred * m_ssao;
	};
}