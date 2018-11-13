#ifndef SSAO_GL_HPP
#define SSAO_GL_HPP

#include <vector>
#include <nex/opengl/texture/Sprite.hpp>
#include <nex/gui/Drawable.hpp>

class TextureGL;
class BaseRenderTargetGL;
class ShaderGL;
class ModelDrawerGL;

const int SSAO_SAMPLING_SIZE = 32;

struct SSAOData {
	float   bias;
	float   intensity;
	float   radius;
	float   _pad0;

	glm::mat4 projection_GPass;

	glm::vec4 samples[SSAO_SAMPLING_SIZE]; // the w component is not used (just for padding)!
};


class SSAO_RendertargetGL : public BaseRenderTargetGL {
public:
	SSAO_RendertargetGL(int width, int height, GLuint frameBuffer, GLuint ssaoColorBuffer);

	virtual ~SSAO_RendertargetGL() = default;

	SSAO_RendertargetGL(SSAO_RendertargetGL&& o);
	SSAO_RendertargetGL& operator=(SSAO_RendertargetGL&& o);

	SSAO_RendertargetGL(const SSAO_RendertargetGL& o) = delete;
	SSAO_RendertargetGL& operator=(const SSAO_RendertargetGL& o) = delete;

	TextureGL* getTexture();

private:
	void swap(SSAO_RendertargetGL& o);

protected:
	TextureGL ssaoColorBuffer;
};




class SSAO_DeferredGL {
public:

	SSAO_DeferredGL(unsigned int windowWidth,
		unsigned int windowHeight, ModelDrawerGL* modelDrawer);

	virtual ~SSAO_DeferredGL() = default;

	TextureGL* getAO_Result();
	TextureGL* getBlurredResult();

	TextureGL* getNoiseTexture();
	void onSizeChange(unsigned int newWidth, unsigned int newHeight);

	void renderAO(TextureGL* gPositions, TextureGL* gNormals, const glm::mat4& projectionGPass);
	void blur();

	void displayAOTexture(TextureGL* aoTexture);

	SSAOData* getSSAOData();

	void setBias(float bias);
	void setItensity(float itensity);
	void setRadius(float radius);


private:

	static SSAO_RendertargetGL createSSAO_FBO(unsigned int width, unsigned int height);

	float randomFloat(float a, float b);
	float lerp(float a, float b, float f);

	TextureGL noiseTexture;
	SSAO_RendertargetGL aoRenderTarget;
	SSAO_RendertargetGL tiledBlurRenderTarget;
	std::unique_ptr<ShaderGL> aoPass;
	std::unique_ptr<ShaderGL> tiledBlurPass;
	std::unique_ptr<ShaderGL> aoDisplay;
	ModelDrawerGL* modelDrawer;

	unsigned int windowWidth;
	unsigned int windowHeight;
	unsigned int noiseTileWidth;
	std::array<glm::vec3, SSAO_SAMPLING_SIZE> ssaoKernel;
	std::vector<glm::vec3> noiseTextureValues;

	Sprite screenSprite;

	SSAOData   m_shaderData;
};

class SSAO_ConfigurationView : public nex::engine::gui::Drawable {
public:
	SSAO_ConfigurationView(SSAO_DeferredGL* ssao);

protected:
	void drawSelf() override;

private:
	SSAO_DeferredGL * m_ssao;
};

#endif //SSAO_GL_HPP