#include <nex/post_processing/SSAO.hpp>
#include <random>
#include <nex/texture/Texture.hpp>
#include <glm/glm.hpp>

using namespace std; 
using namespace glm;


SSAO_Deferred::SSAO_Deferred(unsigned int windowWidth,
	unsigned int windowHeight, 
	unsigned int kernelSize, 
	unsigned int noiseTileWidth) 
	:
	windowWidth(windowWidth),
	windowHeight(windowHeight),
	kernelSize(kernelSize), 
	noiseTileWidth(noiseTileWidth)
{
	// create random kernel samples
	for (unsigned int i = 0; i < kernelSize; ++i) {
		vec3 vec;
		vec.x = randomFloat(-1, 1);
		vec.y = randomFloat(-1, 1);
		vec.z = randomFloat(0, 1);

		if (vec.length() != 0)
			normalize(vec);

		//vec *= randomFloat(0, 1);

		float scale = i / (float)kernelSize;
		scale = lerp(0.1f, 1.0f, scale * scale);
		//vec *= scale;

		ssaoKernel.emplace_back(move(vec));
	}

	//create noise texture (random rotation vectors in tangent space)
	for (unsigned int i = 0; i < noiseTileWidth * noiseTileWidth; ++i) {
		vec3 vec;
		vec.x = randomFloat(-1, 1);
		vec.y = randomFloat(-1, 1);
		vec.z = 0.0f; // we rotate on z-axis (tangent space); thus no z-component needed

		noiseTextureValues.emplace_back(move(vec));
	}
}

float SSAO_Deferred::randomFloat(float a, float b) {
	uniform_real_distribution<float> dist(a, b);
	random_device device;
	default_random_engine gen(device());
	return dist(gen);
}

float SSAO_Deferred::lerp(float a, float b, float f) {
	return a + f*(b - a);
}