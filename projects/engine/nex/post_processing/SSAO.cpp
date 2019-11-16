#include <nex/texture/Texture.hpp>
#include <nex/post_processing/SSAO.hpp>
#include <vector>
#include <nex/util/ExceptionHandling.hpp>
#include <random>
#include <imgui/imgui.h>
#include <nex/gui/Util.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/drawing/MeshDrawer.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include "nex/texture/Attachment.hpp"
#include "nex/texture/Sprite.hpp"
#include <nex/shader/Shader.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/texture/Sampler.hpp>
#include <nex/material/Material.hpp>

using namespace std; 
using namespace glm;

namespace nex
{


	class SsaoPass : public nex::Shader
	{
	public:

		SsaoPass(unsigned int kernelSampleSize) :
			mSamplerDepth(SamplerDesc()), mSamplerNoise(SamplerDesc()),
			kernelSampleSize(kernelSampleSize), m_ssaoData(nullptr), m_texNoise(nullptr), m_gDepth(nullptr), m_samples(nullptr),
			m_ssaoUBO(0, sizeof(SSAOData), nullptr, ShaderBuffer::UsageHint::DYNAMIC_COPY)
		{
			mProgram = ShaderProgram::create("post_processing/ssao/fullscreenquad.vert.glsl",
			                                 "post_processing/ssao/ssao_deferred_ao_fs.glsl");

			mSamplerDepth.setMinFilter(TexFilter::Linear);
			mSamplerDepth.setMagFilter(TexFilter::Linear);
			mSamplerDepth.setWrapR(UVTechnique::ClampToBorder);
			mSamplerDepth.setWrapS(UVTechnique::ClampToBorder);
			mSamplerDepth.setWrapT(UVTechnique::ClampToBorder);
			mSamplerDepth.setBorderColor(glm::vec4(1.0));

			mSamplerNoise.setMinFilter(TexFilter::Nearest);
			mSamplerNoise.setMagFilter(TexFilter::Nearest);
			mSamplerNoise.setWrapR(UVTechnique::Repeat);
			mSamplerNoise.setWrapS(UVTechnique::Repeat);
			mSamplerNoise.setWrapT(UVTechnique::Repeat);

			UniformLocation depthLoc = mProgram->getUniformLocation("gDepth");
			mProgram->setBinding(depthLoc, 0);

			UniformLocation noiseLoc = mProgram->getUniformLocation("texNoise");
			mProgram->setBinding(noiseLoc, 1);


		}

		void drawCustom()
		{
			bind();
			
			m_ssaoUBO.bind();
			m_ssaoUBO.update(
				sizeof(SSAOData),//4 * 4 + 4 * 4 + 2 * 64, // we update only the first members and exclude the samples
				m_ssaoData,
				0);

			mProgram->setTexture(m_gDepth, &mSamplerDepth, 0);
			mProgram->setTexture(m_texNoise, &mSamplerNoise, 1);

			const auto& state = RenderState::getNoDepthTest();
			MeshDrawer::drawFullscreenTriangle(state, this);

			mSamplerDepth.unbind(0);
			mSamplerNoise.unbind(1);
		}

		void setGNoiseTexture(Texture* texture) {
			m_texNoise = texture;
		}

		void setGDepthTexture(Texture* texture) {
			m_gDepth = texture;
		}

		void setKernelSamples(const vector<vec3>& vec) {
			assert(kernelSampleSize == vec.size());

			m_samples = &vec;
		}

		void setSSAOData(SSAOData* data)
		{
			m_ssaoData = data;

			bind();
			m_ssaoUBO.bind();
			m_ssaoUBO.update(sizeof(SSAOData), m_ssaoData, 0);
		}



	private:
		unsigned int kernelSampleSize;
		SSAOData* m_ssaoData;
		UniformBuffer m_ssaoUBO;
		Texture* m_texNoise;
		Texture* m_gDepth;
		const std::vector<vec3>* m_samples;
		Sampler mSamplerDepth;
		Sampler mSamplerNoise;
	};

	class SSAO_Tiled_Blur_Shader : public Shader
	{
	public:

		SSAO_Tiled_Blur_Shader() : mBlurSampler(SamplerDesc()){

			mProgram = ShaderProgram::create("post_processing/ssao/ssao_tiled_blur_vs.glsl",
				"post_processing/ssao/ssao_tiled_blur_fs.glsl");

			mAoTexture = { mProgram->getUniformLocation("ssaoInput"), UniformType::TEXTURE2D, 0 };
			mProgram->setBinding(mAoTexture.location, mAoTexture.bindingSlot);

			mBlurSampler.setWrapR(UVTechnique::ClampToBorder);
			mBlurSampler.setWrapS(UVTechnique::ClampToBorder);
			mBlurSampler.setWrapT(UVTechnique::ClampToBorder);
			mBlurSampler.setBorderColor(glm::vec4(1.0));
			mBlurSampler.setMinFilter(TexFilter::Linear);
			mBlurSampler.setMagFilter(TexFilter::Linear);
		}

		virtual ~SSAO_Tiled_Blur_Shader() = default;

		void afterBlur()
		{
			mBlurSampler.unbind(mAoTexture.bindingSlot);
		}

		void setAOTexture(const Texture* texture) {
			mProgram->setTexture(texture, &mBlurSampler, mAoTexture.bindingSlot);
		}

	private:
		UniformTex mAoTexture;
		Sampler mBlurSampler;
	};


	class SSAO_AO_Display_Shader : public Shader
	{
	public:

		SSAO_AO_Display_Shader() {
			mProgram = ShaderProgram::create("post_processing/ssao/ssao_ao_display_vs.glsl",
				"post_processing/ssao/ssao_ao_display_fs.glsl");

			mScreenTexture = { mProgram->getUniformLocation("screenTexture"), UniformType::TEXTURE2D, 0 };
			mProgram->setBinding(mScreenTexture.location, mScreenTexture.bindingSlot);
		}

		virtual ~SSAO_AO_Display_Shader() = default;

		void setScreenTexture(const Texture* texture) {
			mProgram->setTexture(texture, Sampler::getLinear(), mScreenTexture.bindingSlot);
		}

	private:
		UniformTex mScreenTexture;
	};


	SSAO_Deferred::SSAO_Deferred(unsigned int windowWidth,
		unsigned int windowHeight)
		:
		windowWidth(windowWidth),
		windowHeight(windowHeight)
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


		TextureDesc data;
		data.internalFormat = InternalFormat::RGB16F;
		data.colorspace = ColorSpace::RGB; // TODO use ColorSpace::R?
		data.minFilter = TexFilter::Nearest;
		data.magFilter = TexFilter::Nearest;
		data.wrapR = UVTechnique::Repeat;
		data.wrapS = UVTechnique::Repeat;
		data.wrapT = UVTechnique::Repeat;
		data.pixelDataType = PixelDataType::FLOAT;
		data.useSwizzle = false;
		data.generateMipMaps = false;
		noiseTexture = make_unique<Texture2D>(noiseTileWidth, noiseTileWidth, data, &noiseTextureValues[0]);

		aoRenderTarget = createSSAO_FBO(windowWidth, windowHeight);
		tiledBlurRenderTarget = createSSAO_FBO(windowWidth, windowHeight);

		aoPass = make_unique <SsaoPass>(32);

		tiledBlurPass = make_unique <SSAO_Tiled_Blur_Shader>();

		aoDisplayPass = make_unique <SSAO_AO_Display_Shader>();


		SsaoPass*  configAO = dynamic_cast<SsaoPass*>(aoPass.get());
		configAO->setGNoiseTexture(noiseTexture.get());

		for (int i = 0; i < SSAO_SAMPLING_SIZE; ++i)
		{
			m_shaderData.samples[i] = vec4(ssaoKernel[i], 0);
		}

		configAO->setSSAOData(&m_shaderData);
	}

	Texture2D * SSAO_Deferred::getAO_Result()
	{
		return aoRenderTarget->getColor0AttachmentTexture();
	}

	Texture2D * SSAO_Deferred::getBlurredResult()
	{
		return tiledBlurRenderTarget->getColor0AttachmentTexture();
	}

	Texture2D * SSAO_Deferred::getNoiseTexture()
	{
		return noiseTexture.get();
	}

	void SSAO_Deferred::onSizeChange(unsigned int newWidth, unsigned int newHeight)
	{
		this->windowWidth = newWidth;
		this->windowHeight = newHeight;
		aoRenderTarget = createSSAO_FBO(newWidth, newHeight);
		tiledBlurRenderTarget = createSSAO_FBO(newWidth, newHeight);
	}

	void SSAO_Deferred::renderAO(Texture * gDepth, const glm::mat4& projectionGPass)
	{
		static auto* renderBackend = RenderBackend::get();

		SsaoPass&  aoShader = dynamic_cast<SsaoPass&>(*aoPass);
		const unsigned width = aoRenderTarget->getWidth();
		const unsigned height = aoRenderTarget->getHeight();

		aoShader.setGDepthTexture(gDepth);
		m_shaderData.projection_GPass = projectionGPass;
		m_shaderData.inv_projection_GPass = inverse(projectionGPass);
		m_shaderData.invFullResolution = glm::vec4(1 / float(width), 1/float(height), 0, 0);

		renderBackend->setViewPort(0, 0, width, height);

		aoRenderTarget->bind();
		aoRenderTarget->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil
		aoShader.drawCustom();
	}

	void SSAO_Deferred::blur()
	{
		SSAO_Tiled_Blur_Shader* tiledBlurShader = reinterpret_cast<SSAO_Tiled_Blur_Shader*>(tiledBlurPass.get());
		tiledBlurShader->bind();
		tiledBlurShader->setAOTexture(aoRenderTarget->getColorAttachments()[0].texture.get());

		static auto* renderBackend = RenderBackend::get();
		renderBackend->setViewPort(0, 0, tiledBlurRenderTarget->getWidth(), tiledBlurRenderTarget->getHeight());
		renderBackend->setScissor(0, 0, tiledBlurRenderTarget->getWidth(), tiledBlurRenderTarget->getHeight());
		tiledBlurRenderTarget->bind();
		tiledBlurRenderTarget->clear(RenderComponent::Color | RenderComponent::Depth); // | RenderComponent::Stencil
		const auto& state = RenderState::getNoDepthTest();
		MeshDrawer::drawFullscreenTriangle(state, tiledBlurShader);

		tiledBlurShader->afterBlur();
	}

	void SSAO_Deferred::displayAOTexture(Texture* aoTexture)
	{
		SSAO_AO_Display_Shader* aoDisplayShader = reinterpret_cast<SSAO_AO_Display_Shader*>(aoDisplayPass.get());

		aoDisplayShader->bind();
		aoDisplayShader->setScreenTexture(aoTexture);
		const auto& state = RenderState::getNoDepthTest();
		MeshDrawer::drawFullscreenTriangle(state, aoDisplayShader);
	}

	std::unique_ptr<RenderTarget2D> SSAO_Deferred::createSSAO_FBO(unsigned int width, unsigned int height)
	{
		TextureDesc data;
		data.internalFormat = InternalFormat::R8;
		data.colorspace = ColorSpace::R; // TODO use ColorSpace::R?
		data.minFilter = TexFilter::Linear;
		data.magFilter = TexFilter::Linear;
		data.wrapR = UVTechnique::ClampToBorder;
		data.wrapS = UVTechnique::ClampToBorder;
		data.wrapT = UVTechnique::ClampToBorder;
		data.pixelDataType = PixelDataType::UBYTE;
		data.useSwizzle = false;
		data.generateMipMaps = false;

		return make_unique<RenderTarget2D>(width, height, data, 1);
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
		return a + f * (b - a);
	}

	SSAO_ConfigurationView::SSAO_ConfigurationView(SSAO_Deferred* ssao) : m_ssao(ssao)
	{
	}

	void SSAO_ConfigurationView::drawSelf()
	{
		// render configuration properties
		ImGui::PushID(mId.c_str());
		ImGui::LabelText("", "SSAO:");

		SSAOData* data = m_ssao->getSSAOData();

		ImGui::SliderFloat("bias", &data->bias, 0.0f, 5.0f);
		ImGui::SliderFloat("intensity", &data->intensity, 0.0f, 10.0f);
		ImGui::SliderFloat("radius", &data->radius, 0.0f, 10.0f);

		ImGui::Dummy(ImVec2(0, 20));
		nex::gui::Separator(2.0f);

		ImGui::PopID();
	}
}