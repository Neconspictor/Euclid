#ifndef SSAO_GL_HPP
#define SSAO_GL_HPP

#include <nex/post_processing/SSAO.hpp>

class TextureGL;
class BaseRenderTargetGL;
class ShaderGL;
class ModelDrawerGL;

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




class SSAO_DeferredGL : public SSAO_Deferred {
public:

	SSAO_DeferredGL(unsigned int windowWidth,
		unsigned int windowHeight, ModelDrawerGL* modelDrawer);

	virtual ~SSAO_DeferredGL() = default;

	virtual Texture* getAO_Result() override;
	virtual Texture* getBlurredResult() override;

	virtual Texture* getNoiseTexture() override;
	virtual void onSizeChange(unsigned int newWidth, unsigned int newHeight) override;

	virtual void renderAO(Texture* gPositions, Texture* gNormals, const glm::mat4& projectionGPass) override;
	virtual void blur() override;

	virtual void displayAOTexture() override;

protected:
	static SSAO_RendertargetGL createSSAO_FBO(unsigned int width, unsigned int height);

protected:
	TextureGL noiseTexture;
	SSAO_RendertargetGL aoRenderTarget;
	SSAO_RendertargetGL tiledBlurRenderTarget;
	std::unique_ptr<ShaderGL> aoPass;
	std::unique_ptr<ShaderGL> tiledBlurPass;
	std::unique_ptr<ShaderGL> aoDisplay;
	ModelDrawerGL* modelDrawer;
};

#endif //SSAO_GL_HPP