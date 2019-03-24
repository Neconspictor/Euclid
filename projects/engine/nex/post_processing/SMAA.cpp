#include <nex/post_processing/SMAA.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/texture/Sampler.hpp>
#include <extern/SMAA/AreaTex.h>
#include <extern/SMAA/SearchTex.h>
#include <nex/shader/Shader.hpp>
#include <nex/texture/TextureManager.hpp>


namespace nex
{
	std::string calcMetricDefine(float width, float height)
	{
		std::stringstream ss;
		ss << "#define SMAA_RT_METRICS float4(" << 1.0 / width << ", " << 1.0 / height << ", " << width << ".0, " << height << ".0)";
			return ss.str();
	}

	class SMAA::EdgeDetectionShader : public nex::Shader {
	public:

		EdgeDetectionShader(unsigned width, unsigned height)
		{

			std::vector<std::string> defines {
				calcMetricDefine(width, height)
			};

			mProgram = ShaderProgram::create("post_processing/SMAA/SMAA_EdgeDetection_vs.glsl", 
				"post_processing/SMAA/SMAA_LumaEdgeDetection_fs.glsl", "", defines);
			mColorTexGamma = {mProgram->getUniformLocation("colorTexGamma"), UniformType::TEXTURE2D, 0};
		}

		void setColorTexGamma(Texture2D* tex)
		{
			mProgram->setTexture(mColorTexGamma.location, tex, mColorTexGamma.location);
		}

	private:

		UniformTex mColorTexGamma;
	};

	class SMAA::BlendingWeightCalculationShader : public nex::Shader {
	public:

		BlendingWeightCalculationShader(unsigned width, unsigned height)
		{
			std::vector<std::string> defines{
				calcMetricDefine(width, height)
			};

			mProgram = ShaderProgram::create("post_processing/SMAA/SMAA_BlendingWeightCalculation_vs.glsl",
				"post_processing/SMAA/SMAA_BlendingWeightCalculation_fs.glsl", "", defines);
			mEdgeTex = { mProgram->getUniformLocation("edgeTex"), UniformType::TEXTURE2D, 0};
			mAreaTex = { mProgram->getUniformLocation("areaTex"), UniformType::TEXTURE2D, 1};
			mSearchTex = { mProgram->getUniformLocation("searchTex"), UniformType::TEXTURE2D, 2};
		}

		void setEdgeTex(Texture2D* tex)
		{
			mProgram->setTexture(mEdgeTex.location, tex, mEdgeTex.location);
		}

		void setAreaTex(Texture2D* tex)
		{
			mProgram->setTexture(mAreaTex.location, tex, mAreaTex.location);
		}

		void setSearchTex(Texture2D* tex)
		{
			mProgram->setTexture(mSearchTex.location, tex, mSearchTex.location);
		}

	private:

		UniformTex mEdgeTex;
		UniformTex mAreaTex;
		UniformTex mSearchTex;
	};


	class SMAA::NeighborhoodBlendingShader : public nex::Shader {
	public:

		NeighborhoodBlendingShader(unsigned width, unsigned height)
		{
			std::vector<std::string> defines{
				calcMetricDefine(width, height)
			};

			mProgram = ShaderProgram::create("post_processing/SMAA/SMAA_NeighborhoodBlending_vs.glsl",
				"post_processing/SMAA/SMAA_NeighborhoodBlending_fs.glsl", "", defines);
			mBlendTex = { mProgram->getUniformLocation("blendTex"), UniformType::TEXTURE2D, 0 };
			mColorTex = { mProgram->getUniformLocation("colorTex"), UniformType::TEXTURE2D, 1 };
		}

		void setBlendTex(Texture2D* tex)
		{
			mProgram->setTexture(mBlendTex.location, tex, mBlendTex.location);
		}

		void setColorTex(Texture2D* tex)
		{
			mProgram->setTexture(mColorTex.location, tex, mColorTex.location);
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
	TextureData areaDesc;
	areaDesc.wrapR = areaDesc.wrapS = areaDesc.wrapT = TextureUVTechnique::ClampToEdge;
	areaDesc.minFilter = areaDesc.magFilter = TextureFilter::Linear;
	areaDesc.colorspace = ColorSpace::RGB;
	areaDesc.internalFormat = InternFormat::RGB8;
	areaDesc.pixelDataType = PixelDataType::UBYTE;

	//flip y axis of areatex
	/*std::vector<char>temp(AREATEX_PITCH * AREATEX_HEIGHT);
	memcpy_s(temp.data(), temp.size(), areaTexBytes, temp.size());
	TextureManager::flipYAxis(temp.data(), AREATEX_PITCH, AREATEX_HEIGHT);

	mAreaTex = std::make_unique<Texture2D>(AREATEX_WIDTH, AREATEX_HEIGHT, areaDesc, temp.data());*/

	mAreaTex = TextureManager::get()->loadImage("_intern/smaa/AreaTexDX10.tga", true, areaDesc);

	TextureData searchDesc;
	searchDesc.wrapR = searchDesc.wrapS = searchDesc.wrapT = TextureUVTechnique::ClampToEdge;
	searchDesc.minFilter = searchDesc.magFilter = TextureFilter::NearestNeighbor;
	searchDesc.colorspace = ColorSpace::RGB;
	searchDesc.internalFormat = InternFormat::RGB8;
	searchDesc.pixelDataType = PixelDataType::UBYTE;

	//flip y axis of searchtex
	/*temp.resize(SEARCHTEX_PITCH * SEARCHTEX_HEIGHT);
	memcpy_s(temp.data(), temp.size(), searchTexBytes, temp.size());
	TextureManager::flipYAxis(temp.data(), SEARCHTEX_PITCH, SEARCHTEX_HEIGHT);

	mSearchTex = std::make_unique<Texture2D>(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, searchDesc, temp.data());*/

	mSearchTex = TextureManager::get()->loadImage("_intern/smaa/SearchTex.tga", true, searchDesc);

	//release memory
	//temp.resize(0);

	SamplerDesc samplerDesc;
	samplerDesc.minFilter = samplerDesc.magFilter = TextureFilter::NearestNeighbor;
	samplerDesc.maxAnisotropy = 0.0f;
	samplerDesc.wrapR = samplerDesc.wrapS = areaDesc.wrapT = TextureUVTechnique::ClampToEdge;
	mPointFilter = std::make_unique<Sampler>(samplerDesc);

	samplerDesc.minFilter = samplerDesc.magFilter = TextureFilter::Linear;
	mBilinearFilter = std::make_unique<Sampler>(samplerDesc);


	mFullscreenTriangle = StaticMeshManager::get()->getNDCFullscreenPlane();
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

void nex::SMAA::resize(unsigned width, unsigned height)
{
	TextureData data;
	data.generateMipMaps = false;
	data.minFilter = data.magFilter = TextureFilter::Linear;
	data.colorspace = ColorSpace::RGBA;
	data.internalFormat = InternFormat::RGBA8;
	data.pixelDataType = PixelDataType::UBYTE;
	data.wrapR = data.wrapS  = data.wrapT = TextureUVTechnique::ClampToEdge;

	mEdgesTex = std::make_unique<RenderTarget2D>(width, height, data);

	data.colorspace = ColorSpace::RGBA;
	data.internalFormat = InternFormat::RGBA8;
	data.pixelDataType = PixelDataType::UBYTE;
	mBlendTex = std::make_unique<RenderTarget2D>(width, height, data);

	mEdgeDetectionShader = std::make_unique<EdgeDetectionShader>(width, height);
	mBlendingWeightCalculationShader = std::make_unique<BlendingWeightCalculationShader>(width, height);
	mNeighborhoodBlendingShader = std::make_unique<NeighborhoodBlendingShader>(width, height);
}

nex::Texture2D* nex::SMAA::renderEdgeDetectionPass(Texture2D* colorTexGamma)
{
	mEdgesTex->bind();
	RenderBackend::get()->setViewPort(0, 0, mEdgesTex->getWidth(), mEdgesTex->getHeight());
	mEdgeDetectionShader->bind();
	mBilinearFilter->bind(0);
	mEdgeDetectionShader->setColorTexGamma(colorTexGamma);

	mFullscreenTriangle->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);

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

	mFullscreenTriangle->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);

	return mBlendTex->getColor0AttachmentTexture();
}

void nex::SMAA::renderNeighborhoodBlendingPass(Texture2D* blendTex, Texture2D* colorTex, RenderTarget2D* output)
{
	output->bind();
	RenderBackend::get()->setViewPort(0, 0, output->getWidth(), output->getHeight());
	output->clear(Color | Depth | Stencil);
	mNeighborhoodBlendingShader->bind();
	mBilinearFilter->bind(0);
	mBilinearFilter->bind(1);
	mNeighborhoodBlendingShader->setBlendTex(blendTex);
	mNeighborhoodBlendingShader->setColorTex(colorTex);

	mFullscreenTriangle->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);
}


void nex::SMAA::reset()
{
	mEdgesTex->bind();
	mEdgesTex->clear(RenderComponent::Color | Depth | Stencil); // no depth-/stencil buffer needed

	mBlendTex->bind();
	mBlendTex->clear(RenderComponent::Color | Depth | Stencil); // no depth-/stencil buffer needed
}