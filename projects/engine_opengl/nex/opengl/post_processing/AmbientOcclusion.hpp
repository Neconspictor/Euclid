#pragma once

#include <nex/opengl/texture/Sprite.hpp>
#include <memory>
#include <nex/opengl/post_processing/HBAO_GL.hpp>
#include <nex/opengl/post_processing/SSAO_GL.hpp>

class SceneNode;
class TextureGL;

class AmbientOcclusion_Deferred {
public:

	AmbientOcclusion_Deferred(unsigned int windowWidth, unsigned int windowHeight) : 
	m_windowWidth(windowWidth), m_windowHeight(windowHeight){}

	virtual ~AmbientOcclusion_Deferred() = default;

	virtual TextureGL* getAO_Result() = 0;
	virtual TextureGL* getBlurredResult() = 0;
	virtual void onSizeChange(unsigned int newWidth, unsigned int newHeight) = 0;

	//virtual void renderAO(Texture* depth, const Projection& projection, bool blur) = 0;
	virtual void blur() = 0;
	virtual void displayAOTexture(TextureGL* aoTexture) = 0;
	
protected:
	unsigned int m_windowWidth;
	unsigned int m_windowHeight;
};

class AmbientOcclusionSelector
{
public:

	enum AOTechnique
	{
		HBAO,
		SSAO
	};

	AmbientOcclusionSelector();
	virtual ~AmbientOcclusionSelector();

	hbao::HBAO_GL* getHBAO();
	SSAO_DeferredGL* getSSAO();

	bool isAmbientOcclusionActive() const;

	AOTechnique getActiveAOTechnique() const;

	void setAOTechniqueToUse(AOTechnique technique);
	void setUseAmbientOcclusion(bool useAO);

	void setHBAO(std::unique_ptr<hbao::HBAO_GL> hbao);
	void setSSAO(std::unique_ptr<SSAO_DeferredGL> ssao);

private:

	std::unique_ptr<hbao::HBAO_GL> m_hbao;
	std::unique_ptr<SSAO_DeferredGL> m_ssao;
	bool m_useAO = true;
	AOTechnique m_usedAOTechnique;
};

std::ostream& operator<<(std::ostream& os, AmbientOcclusionSelector::AOTechnique aoTechnique);