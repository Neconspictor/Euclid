#ifndef SSAO_GL_HPP
#define SSAO_GL_HPP

#include <vector>
#include <nex/texture/Sprite.hpp>
#include <nex/gui/Drawable.hpp>

namespace nex
{

	class Texture;
	class RenderTarget;
	class RenderTarget2D;
	class ShaderProgram;
	class StaticMeshDrawer;

	const int SSAO_SAMPLING_SIZE = 32;

	struct SSAOData {
		float   bias;
		float   intensity;
		float   radius;
		float   _pad0;

		glm::mat4 projection_GPass;

		glm::vec4 samples[SSAO_SAMPLING_SIZE]; // the w component is not used (just for padding)!
	};


	class SSAO_DeferredGL {
	public:

		SSAO_DeferredGL(unsigned int windowWidth,
			unsigned int windowHeight, StaticMeshDrawer* modelDrawer);

		virtual ~SSAO_DeferredGL() = default;

		Texture* getAO_Result();
		Texture* getBlurredResult();

		Texture* getNoiseTexture();
		void onSizeChange(unsigned int newWidth, unsigned int newHeight);

		void renderAO(Texture* gPositions, Texture* gNormals, const glm::mat4& projectionGPass);
		void blur();

		void displayAOTexture(Texture* aoTexture);

		SSAOData* getSSAOData();

		void setBias(float bias);
		void setItensity(float itensity);
		void setRadius(float radius);


	private:

		static std::unique_ptr<RenderTarget2D> createSSAO_FBO(unsigned int width, unsigned int height);

		float randomFloat(float a, float b);
		float lerp(float a, float b, float f);

		std::unique_ptr<Texture> noiseTexture;
		std::unique_ptr<RenderTarget2D> aoRenderTarget;
		std::unique_ptr<RenderTarget2D> tiledBlurRenderTarget;
		std::unique_ptr<nex::Shader> aoPass;
		std::unique_ptr<nex::Shader> tiledBlurPass;
		std::unique_ptr<nex::Shader> aoDisplay;
		StaticMeshDrawer* modelDrawer;

		unsigned int windowWidth;
		unsigned int windowHeight;
		unsigned int noiseTileWidth;
		std::array<glm::vec3, SSAO_SAMPLING_SIZE> ssaoKernel;
		std::vector<glm::vec3> noiseTextureValues;

		nex::Sprite screenSprite;

		SSAOData   m_shaderData;
	};

	class SSAO_ConfigurationView : public nex::gui::Drawable {
	public:
		SSAO_ConfigurationView(SSAO_DeferredGL* ssao);

	protected:
		void drawSelf() override;

	private:
		SSAO_DeferredGL * m_ssao;
	};

}

#endif //SSAO_GL_HPP