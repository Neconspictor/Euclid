#include <nex/shadow/ShadowMap.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/math/BoundingBox.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/light/Light.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/texture/Image.hpp>
#include <nex/gui/Util.hpp>

nex::ShadowMap::DepthPass::DepthPass() : TransformPass(Shader::create("shadow/shadow_map_depth_vs.glsl", "shadow/shadow_map_depth_fs.glsl"))
{

}

nex::ShadowMap::ShadowMap(unsigned int width, unsigned int height, const PCFFilter& pcf, float biasMultiplier, float shadowStrength) :
	mPCF(pcf), mBiasMultiplier(biasMultiplier), mShadowStrength(shadowStrength), mDepthPass(std::make_unique<DepthPass>())
{
	resize(width, height);
}

void nex::ShadowMap::resize(unsigned int width, unsigned int height)
{
	mRenderTarget = std::make_unique<RenderTarget>(width, height);

	TextureDesc data;
	data.colorspace = ColorSpace::DEPTH;
	data.internalFormat = InternalFormat::DEPTH32;
	data.pixelDataType = PixelDataType::FLOAT;
	data.minFilter = TexFilter::Nearest; // IMPORTANT: Linear filter produces ugly artifacts when using PCF filtering
	data.magFilter = TexFilter::Nearest; // IMPORTANT: Linear filter produces ugly artifacts when using PCF filtering
	data.wrapR = data.wrapS = data.wrapT = UVTechnique::ClampToBorder;
	data.borderColor = glm::vec4(1.0f);
	//data.useDepthComparison = true;
	data.compareFunction = CompFunc::LESS;
	data.useSwizzle = true;
	data.swizzle = { Channel::RED, Channel::RED, Channel::RED, Channel::ALPHA };

	RenderAttachment depth;
	depth.type = nex::RenderAttachmentType::DEPTH;
	depth.target = TextureTarget::RENDER_BUFFER;
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

const nex::PCFFilter& nex::ShadowMap::getPCF() const
{
	return mPCF;
}

nex::Texture* nex::ShadowMap::getRenderResult()
{
	return mRenderTarget->getDepthAttachment()->texture.get();
}

const nex::Texture* nex::ShadowMap::getRenderResult() const
{
	return mRenderTarget->getDepthAttachment()->texture.get();
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

const glm::mat4& nex::ShadowMap::getViewProjection() const
{
	return mViewProj;
}

void nex::ShadowMap::render(const nex::RenderCommandQueue::Buffer& shadowCommands)
{
	
	RenderBackend::get()->setViewPort(0, 0, mRenderTarget->getWidth(), mRenderTarget->getHeight());
	mRenderTarget->bind();
	auto* depthBuffer = RenderBackend::get()->getDepthBuffer();
	depthBuffer->enableDepthBufferWriting(true);
	depthBuffer->enableDepthTest(true);
	mRenderTarget->clear(RenderComponent::Depth | RenderComponent::Color | RenderComponent::Stencil);

	mDepthPass->bind();
	mDepthPass->setViewProjectionMatrices(mProjection, mView, mView, mProjection);

	for (const auto& command : shadowCommands)
	{
		mDepthPass->setModelMatrix(command.worldTrafo, command.prevWorldTrafo);
		mDepthPass->uploadTransformMatrices();
		StaticMeshDrawer::draw(command.mesh, nullptr);
	}
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
	const auto radius = length(shadowBounds.max - shadowBounds.min) / 2.0f;

	glm::vec3 up(0,1,0);

	const auto angle = dot(look, up);

	if (abs(angle) < EPS) {
		up = glm::vec3(1,0,0);
	}

	const auto lightPosition = center - look * radius;
		
	mView = glm::lookAt(lightPosition, center, up);
	
	auto extents = mView * shadowBounds;

	extents.min.z = abs(extents.min.z);
	extents.max.z = abs(extents.max.z);
	auto minBackup = extents.min.z;
	auto maxBackup = extents.max.z;
	extents.min.z = std::min<float>(minBackup, maxBackup);
	extents.max.z = std::max<float>(minBackup, maxBackup);

	mProjection = glm::ortho(extents.min.x, extents.max.x, //extents.min.x, extents.max.x
							extents.min.y, extents.max.y,  //extents.min.y, extents.max.y
							extents.min.z, extents.max.z); //extents.min.z, extents.max.z

	mViewProj = mProjection * mView;
}

nex::ShadowMap_ConfigurationView::ShadowMap_ConfigurationView(const nex::ShadowMap* model) : 
	mModel(model), mTextureView({}, ImVec2(256, 256))
{
}

void nex::ShadowMap_ConfigurationView::drawSelf()
{
	auto* texture = mModel->getRenderResult();
	auto& imageDesc = mTextureView.getTexture();
	imageDesc.texture = texture;
	imageDesc.flipY = ImageFactory::isYFlipped();
	imageDesc.sampler = nullptr;


	mTextureView.updateTexture(true);
	mTextureView.drawGUI();

	nex::gui::Separator(2.0f);
}
