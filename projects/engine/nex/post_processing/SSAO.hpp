#ifndef SSAO_HPP
#define SSAO_HPP

#include <vector>
#include <nex/sprite/Sprite.hpp>

class RenderTarget;
class PBR_GBuffer;
class SceneNode;
class Texture;

class SSAO_Deferred {
public:

	SSAO_Deferred(unsigned int windowWidth,
		unsigned int windowHeight,
		unsigned int kernelSize, 
		unsigned int noiseTileWidth);

	virtual ~SSAO_Deferred() = default;

	virtual Texture* getAO_Result() = 0;
	virtual Texture* getBlurredResult() = 0;
	virtual Texture* getNoiseTexture() = 0;
	virtual void onSizeChange(unsigned int newWidth, unsigned int newHeight) = 0;

	virtual void renderAO(Texture* gPositions, Texture* gNormals, const glm::mat4& projectionGPass) = 0;
	virtual void blur() = 0;
	virtual void displayAOTexture(Texture* aoTexture) = 0;


protected:
	float randomFloat(float a, float b);
	float lerp(float a, float b, float f);
	
protected:
	unsigned int windowWidth;
	unsigned int windowHeight;
	unsigned int kernelSize;
	unsigned int noiseTileWidth;
	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> noiseTextureValues;

	Sprite screenSprite;
};

#endif //SSAO_HPP