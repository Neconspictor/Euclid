#include <nex/post_processing/SMAA.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/Sampler.hpp>
#include <extern/SMAA/AreaTex.h>
#include <extern/SMAA/SearchTex.h>
#include <nex/shader/Shader.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/material/Material.hpp>
#include <nex/mesh/MeshManager.hpp>
#include "nex/renderer/Drawer.hpp"
#include "nex/resource/ResourceLoader.hpp"


namespace nex
{
	std::string calcMetricDefine(float width, float height)
	{
		std::stringstream ss;
		ss << "#define SMAA_RT_METRICS float4(" << 1.0 / width << ", " << 1.0 / height << ", " << width << ".0, " << height << ".0)";
			return ss.str();
	}

	class SMAA::EdgeDetectionPass : public nex::Shader {
	public:

		EdgeDetectionPass(unsigned width, unsigned height)
		{

			std::vector<std::string> defines {
				calcMetricDefine(width, height)
			};

			mProgram = ShaderProgram::create("post_processing/SMAA/SMAA_EdgeDetection_vs.glsl", 
				"post_processing/SMAA/SMAA_ColorEdgeDetection_fs.glsl", nullptr, nullptr, nullptr, defines);
			mColorTexGamma = {mProgram->getUniformLocation("colorTexGamma"), UniformType::TEXTURE2D, 0};

			mProgram->setBinding(mColorTexGamma.location, mColorTexGamma.bindingSlot);
		}

		void setColorTexGamma(Texture2D* tex)
		{
			mProgram->setTexture(tex, nullptr, mColorTexGamma.bindingSlot);
		}

	private:

		UniformTex mColorTexGamma;
	};

	class SMAA::BlendingWeightCalculationPass : public nex::Shader {
	public:

		BlendingWeightCalculationPass(unsigned width, unsigned height)
		{
			std::vector<std::string> defines{
				calcMetricDefine(width, height)
			};

			mProgram = ShaderProgram::create("post_processing/SMAA/SMAA_BlendingWeightCalculation_vs.glsl",
				"post_processing/SMAA/SMAA_BlendingWeightCalculation_fs.glsl", nullptr, nullptr, nullptr, defines);
			mEdgeTex = { mProgram->getUniformLocation("edgeTex"), UniformType::TEXTURE2D, 0};
			mAreaTex = { mProgram->getUniformLocation("areaTex"), UniformType::TEXTURE2D, 1};
			mSearchTex = { mProgram->getUniformLocation("searchTex"), UniformType::TEXTURE2D, 2};

			mProgram->setBinding(mEdgeTex.location, mEdgeTex.bindingSlot);
			mProgram->setBinding(mAreaTex.location, mAreaTex.bindingSlot);
			mProgram->setBinding(mSearchTex.location, mSearchTex.bindingSlot);
		}

		void setEdgeTex(Texture2D* tex)
		{
			mProgram->setTexture(tex, nullptr, mEdgeTex.bindingSlot);
		}

		void setAreaTex(Texture2D* tex)
		{
			mProgram->setTexture(tex, nullptr, mAreaTex.bindingSlot);
		}

		void setSearchTex(Texture2D* tex)
		{
			mProgram->setTexture(tex, nullptr, mSearchTex.bindingSlot);
		}

	private:

		UniformTex mEdgeTex;
		UniformTex mAreaTex;
		UniformTex mSearchTex;
	};


	class SMAA::NeighborhoodBlendingPass : public nex::Shader {
	public:

		NeighborhoodBlendingPass(unsigned width, unsigned height)
		{
			std::vector<std::string> defines{
				calcMetricDefine(width, height)
			};

			mProgram = ShaderProgram::create("post_processing/SMAA/SMAA_NeighborhoodBlending_vs.glsl",
				"post_processing/SMAA/SMAA_NeighborhoodBlending_fs.glsl", nullptr, nullptr, nullptr, defines);
			mBlendTex = { mProgram->getUniformLocation("blendTex"), UniformType::TEXTURE2D, 0 };
			mColorTex = { mProgram->getUniformLocation("colorTex"), UniformType::TEXTURE2D, 1 };

			mProgram->setBinding(mBlendTex.location, mBlendTex.bindingSlot);
			mProgram->setBinding(mColorTex.location, mColorTex.bindingSlot);
		}

		void setBlendTex(Texture2D* tex)
		{
			mProgram->setTexture(tex, nullptr, mBlendTex.bindingSlot);
		}

		void setColorTex(Texture2D* tex)
		{
			mProgram->setTexture(tex, nullptr, mColorTex.bindingSlot);
		}

	private:

		UniformTex mBlendTex;
		UniformTex mColorTex;
	};
}


nex::SMAA::SMAA(unsigned width, unsigned height)
{
	resize(width, height);

	// TODO load area and search textures

	//flip y axis of areatex
	/*std::vector<char>temp(AREATEX_PITCH * AREATEX_HEIGHT);
	memcpy_s(temp.data(), temp.size(), areaTexBytes, temp.size());
	TextureManager::flipYAxis(temp.data(), AREATEX_PITCH, AREATEX_HEIGHT);

	mAreaTex = std::make_unique<Texture2D>(AREATEX_WIDTH, AREATEX_HEIGHT, areaDesc, temp.data());*/





	TextureDesc areaDesc;
	areaDesc.wrapR = areaDesc.wrapS = areaDesc.wrapT = UVTechnique::ClampToEdge;
	areaDesc.minFilter = areaDesc.magFilter = TexFilter::Linear;
	areaDesc.internalFormat = InternalFormat::RGB8;
	mAreaTex = TextureManager::get()->loadImage("_intern/smaa/AreaTexDX10.tga", true, areaDesc);

	TextureDesc searchDesc;
	searchDesc.wrapR = searchDesc.wrapS = searchDesc.wrapT = UVTechnique::ClampToEdge;
	searchDesc.minFilter = searchDesc.magFilter = TexFilter::Nearest;
	searchDesc.internalFormat = InternalFormat::RGB8;

	mSearchTex = TextureManager::get()->loadImage("_intern/smaa/SearchTex.tga", true, searchDesc);

	SamplerDesc samplerDesc;
	samplerDesc.minFilter = samplerDesc.magFilter = TexFilter::Nearest;
	samplerDesc.maxAnisotropy = 1.0f;
	samplerDesc.wrapR = samplerDesc.wrapS = samplerDesc.wrapT = UVTechnique::ClampToEdge;
	mPointFilter = std::make_unique<Sampler>(samplerDesc);

	samplerDesc.minFilter = samplerDesc.magFilter = TexFilter::Linear;
	mBilinearFilter = std::make_unique<Sampler>(samplerDesc);
}

nex::SMAA::~SMAA() = default;

nex::Texture2D* nex::SMAA::getArexTex()
{
	return mAreaTex.get();
}

nex::Texture2D* nex::SMAA::getSearchTex()
{
	return mSearchTex.get();
}

nex::Texture2D* nex::SMAA::getEdgeDetection()
{
	return (Texture2D*)mEdgesTex->getColorAttachmentTexture(0);
}

nex::Texture2D* nex::SMAA::getBlendingWeight()
{
	return (Texture2D*)mBlendTex->getColorAttachmentTexture(0);
}

void nex::SMAA::resize(unsigned width, unsigned height)
{
	TextureDesc data;
	data.generateMipMaps = false;
	data.minFilter = data.magFilter = TexFilter::Linear;
	data.internalFormat = InternalFormat::RGBA8;
	data.wrapR = data.wrapS  = data.wrapT = UVTechnique::ClampToEdge;

	mEdgesTex = std::make_unique<RenderTarget2D>(width, height, data);

	data.internalFormat = InternalFormat::RGBA8;
	mBlendTex = std::make_unique<RenderTarget2D>(width, height, data);

	mEdgeDetectionShader = std::make_unique<EdgeDetectionPass>(width, height);
	mBlendingWeightCalculationShader = std::make_unique<BlendingWeightCalculationPass>(width, height);
	mNeighborhoodBlendingShader = std::make_unique<NeighborhoodBlendingPass>(width, height);
}

nex::Texture2D* nex::SMAA::renderEdgeDetectionPass(Texture2D* colorTexGamma)
{
	mEdgesTex->bind();
	RenderBackend::get()->setViewPort(0, 0, mEdgesTex->getWidth(), mEdgesTex->getHeight());
	mEdgeDetectionShader->bind();
	mBilinearFilter->bind(0);
	mEdgeDetectionShader->setColorTexGamma(colorTexGamma);

	const auto& state = RenderState::getNoDepthTest();
	Drawer::drawFullscreenTriangle(state, mEdgeDetectionShader.get());

	return mEdgesTex->getColor0AttachmentTexture();
}

nex::Texture2D* nex::SMAA::renderBlendingWeigthCalculationPass(Texture2D* edgeTex)
{
	mBlendTex->bind();
	RenderBackend::get()->setViewPort(0, 0, mBlendTex->getWidth(), mBlendTex->getHeight());
	mBlendingWeightCalculationShader->bind();
	mBilinearFilter->bind(0);
	mBilinearFilter->bind(1);
	mBilinearFilter->bind(2);
	mBlendingWeightCalculationShader->setEdgeTex(edgeTex);
	mBlendingWeightCalculationShader->setAreaTex(mAreaTex.get());
	mBlendingWeightCalculationShader->setSearchTex(mSearchTex.get());

	const auto& state = RenderState::getNoDepthTest();
	Drawer::drawFullscreenTriangle(state, mBlendingWeightCalculationShader.get());

	return mBlendTex->getColor0AttachmentTexture();
}

void nex::SMAA::renderNeighborhoodBlendingPass(Texture2D* blendTex, Texture2D* colorTex, RenderTarget* output)
{
	output->bind();
	RenderBackend::get()->setViewPort(0, 0, static_cast<int>(output->getWidth()), static_cast<int>(output->getHeight()));
	//LOG(Logger(), Debug) << "width = " << output->getWidth() << ", height = " << output->getHeight();
	output->clear(Color | Depth | Stencil);
	mNeighborhoodBlendingShader->bind();
	mBilinearFilter->bind(0);
	mBilinearFilter->bind(1);
	mNeighborhoodBlendingShader->setBlendTex(blendTex);
	mNeighborhoodBlendingShader->setColorTex(colorTex);

	const auto& state = RenderState::getNoDepthTest();
	Drawer::drawFullscreenTriangle(state, mNeighborhoodBlendingShader.get());
}


void nex::SMAA::reset()
{
	mEdgesTex->bind();
	mEdgesTex->clear(RenderComponent::Color | Depth | Stencil); // no depth-/stencil buffer needed

	mBlendTex->bind();
	mBlendTex->clear(RenderComponent::Color | Depth | Stencil); // no depth-/stencil buffer needed
}