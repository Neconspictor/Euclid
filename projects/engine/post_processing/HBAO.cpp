#include <post_processing/HBAO.hpp>
#include <random>
#include <texture/Texture.hpp>
#include <glm/glm.hpp>

using namespace std; 
using namespace glm;


hbao::HBAO_Deferred::HBAO_Deferred(unsigned int windowWidth,
	unsigned int windowHeight) 
	:
	windowWidth(windowWidth),
	windowHeight(windowHeight)
{
}

float hbao::HBAO_Deferred::randomFloat(float a, float b) {
	uniform_real_distribution<float> dist(a, b);
	random_device device;
	default_random_engine gen(device());
	return dist(gen);
}

float hbao::HBAO_Deferred::lerp(float a, float b, float f) {
	return a + f*(b - a);
}