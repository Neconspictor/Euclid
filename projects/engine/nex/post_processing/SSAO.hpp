#pragma once

#include <vector>
#include <nex/sprite/Sprite.hpp>
#include "nex/gui/Drawable.hpp"

class RenderTarget;
class PBR_GBuffer;
class SceneNode;
class Texture;

const int SSAO_SAMPLING_SIZE = 32;

struct SSAOData {
	float   bias;
	float   intensity;
	float   radius;
	float   _pad0;

	glm::mat4 projection_GPass;

	glm::vec4 samples[SSAO_SAMPLING_SIZE]; // the w component is not used (just for padding)!
};

class SSAO_Deferred {
public:

	SSAO_Deferred(unsigned int windowWidth,
		unsigned int windowHeight,
		unsigned int noiseTileWidth);

	virtual ~SSAO_Deferred() = default;

	virtual Texture* getAO_Result() = 0;
	virtual Texture* getBlurredResult() = 0;
	virtual Texture* getNoiseTexture() = 0;
	virtual void onSizeChange(unsigned int newWidth, unsigned int newHeight) = 0;

	virtual void renderAO(Texture* gPositions, Texture* gNormals, const glm::mat4& projectionGPass) = 0;
	virtual void blur() = 0;
	virtual void displayAOTexture(Texture* aoTexture) = 0;

	SSAOData* getSSAOData();

	virtual void setBias(float bias);
	virtual void setItensity(float itensity);
	virtual void setRadius(float radius);


protected:
	float randomFloat(float a, float b);
	float lerp(float a, float b, float f);
	
protected:
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
	SSAO_ConfigurationView(SSAO_Deferred* ssao);

protected:
	void drawSelf() override;

private:
	SSAO_Deferred * m_ssao;
};