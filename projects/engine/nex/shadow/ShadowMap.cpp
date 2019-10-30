#include <nex/shadow/ShadowMap.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/math/BoundingBox.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/light/Light.hpp>

nex::ShadowMap::ShadowMap(unsigned int width, unsigned int height, const PCFFilter& pcf, float biasMultiplier, float shadowStrength) :
	mPCF(pcf), mBiasMultiplier(biasMultiplier), mShadowStrength(shadowStrength)
{
	resize(width, height);
}

nex::Texture* nex::ShadowMap::getDepthTexture()
{
	return mRenderTarget->getColorAttachmentTexture(0);
}

void nex::ShadowMap::resize(unsigned int width, unsigned int height)
{
	mRenderTarget = std::make_unique<RenderTarget>(width, height);

	TextureDesc data;
	data.colorspace = ColorSpace::DEPTH;
	data.internalFormat = InternalFormat::DEPTH16;
	data.pixelDataType = PixelDataType::UNSIGNED_SHORT;
	data.minFilter = TexFilter::Nearest; // IMPORTANT: Linear filter produces ugly artifacts when using PCF filtering
	data.magFilter = TexFilter::Nearest; // IMPORTANT: Linear filter produces ugly artifacts when using PCF filtering
	data.wrapR = data.wrapS = data.wrapT = UVTechnique::ClampToBorder;
	data.borderColor = glm::vec4(1.0f);
	//data.useDepthComparison = true;
	data.compareFunc = CompFunc::LESS;

	RenderAttachment depth;
	depth.type = nex::RenderAttachmentType::DEPTH;
	depth.target = TextureTarget::TEXTURE2D;
	depth.texture = std::make_unique<Texture2D>(width, height, data, nullptr);

	mRenderTarget->bind();
	mRenderTarget->useDepthAttachment(std::move(depth));
	mRenderTarget->finalizeAttachments();
	mRenderTarget->assertCompletion();
}

nex::TransformPass* nex::ShadowMap::getDepthPass()
{
	return mDepthPass.get();
}

unsigned nex::ShadowMap::getHeight() const
{
	return mRenderTarget->getHeight();
}

const nex::ShadowMap::PCFFilter& nex::ShadowMap::getPCF() const
{
	return mPCF;
}

float nex::ShadowMap::getShadowStrength() const
{
	return mShadowStrength;
}

unsigned nex::ShadowMap::getWidth() const
{
	return mRenderTarget->getWidth();
}

const glm::mat4& nex::ShadowMap::getView() const
{
	return mView;
}

const glm::mat4& nex::ShadowMap::getProjection() const
{
	return mProjection;
}

void nex::ShadowMap::setBiasMultiplier(float bias)
{
	mBiasMultiplier = bias;
}

void nex::ShadowMap::setPCF(const PCFFilter& filter)
{
	mPCF = filter;
}

void nex::ShadowMap::setShadowStrength(float strength)
{
	mShadowStrength = strength;
}

void nex::ShadowMap::update(const DirLight& dirLight, const AABB& shadowBounds)
{
	constexpr auto EPS = 0.001f;
	const auto center = (shadowBounds.max + shadowBounds.min) / 2.0f;
	const auto look = normalize(dirLight.directionWorld);

	glm::vec3 up(0,1,0);

	const auto angle = dot(look, up);

	if (abs(angle) < EPS) {
		up = glm::vec3(1,0,0);
	}

	const auto lightPosition = center - look;
		
	mView = glm::lookAt(lightPosition, center, up);
	
	const auto extents = mView * shadowBounds;

	mProjection = glm::ortho(extents.min.x, extents.max.x, 
							extents.min.y, extents.max.y, 
							extents.min.z, extents.max.z);
}