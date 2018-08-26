#include <nex/post_processing/SSAO.hpp>
#include <random>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <nex/gui/Util.hpp>

using namespace std; 
using namespace glm;


SSAO_Deferred::SSAO_Deferred(unsigned int windowWidth,
	unsigned int windowHeight,
	unsigned int noiseTileWidth) 
	:
	windowWidth(windowWidth),
	windowHeight(windowHeight),
	noiseTileWidth(noiseTileWidth)
{
	// create random kernel samples
	for (unsigned int i = 0; i < SSAO_SAMPLING_SIZE; ++i) {
		vec3 vec;
		vec.x = randomFloat(-1, 1);
		vec.y = randomFloat(-1, 1);
		vec.z = randomFloat(0, 1);

		if (vec.length() != 0)
			normalize(vec);

		//vec *= randomFloat(0, 1);

		float scale = i / (float)SSAO_SAMPLING_SIZE;
		scale = lerp(0.1f, 1.0f, scale * scale);
		//vec *= scale;

		ssaoKernel[i] = move(vec);
	}

	//create noise texture (random rotation vectors in tangent space)
	for (unsigned int i = 0; i < noiseTileWidth * noiseTileWidth; ++i) {
		vec3 vec;
		vec.x = randomFloat(-1, 1);
		vec.y = randomFloat(-1, 1);
		vec.z = 0.0f; // we rotate on z-axis (tangent space); thus no z-component needed

		noiseTextureValues.emplace_back(move(vec));
	}

	m_shaderData.bias = 0.025f;
	m_shaderData.intensity = 1.0f;
	m_shaderData.radius = 0.25f;
}

SSAOData* SSAO_Deferred::getSSAOData()
{
	return &m_shaderData;
}

void SSAO_Deferred::setBias(float bias)
{
	m_shaderData.bias = bias;
}

void SSAO_Deferred::setItensity(float itensity)
{
	m_shaderData.intensity = itensity;
}

void SSAO_Deferred::setRadius(float radius)
{
	m_shaderData.radius = radius;
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

SSAO_ConfigurationView::SSAO_ConfigurationView(SSAO_Deferred* ssao) : m_ssao(ssao)
{
}

void SSAO_ConfigurationView::drawSelf()
{
	// render configuration properties
	ImGui::PushID(m_id.c_str());
	ImGui::LabelText("", "SSAO:");

	SSAOData* data = m_ssao->getSSAOData();

	ImGui::SliderFloat("bias", &data->bias, 0.0f, 5.0f);
	ImGui::SliderFloat("intensity", &data->intensity, 0.0f, 10.0f);
	ImGui::SliderFloat("radius", &data->radius, 0.0f, 10.0f);

	ImGui::Dummy(ImVec2(0, 20));
	nex::engine::gui::Separator(2.0f);

	ImGui::PopID();
}