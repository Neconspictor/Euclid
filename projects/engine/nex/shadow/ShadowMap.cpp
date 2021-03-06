#include <nex/shadow/ShadowMap.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/math/BoundingBox.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/light/Light.hpp>
#include <nex/renderer/Drawer.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/texture/Image.hpp>
#include <nex/gui/ImGUI_Extension.hpp>
#include <nex/mesh/MeshGroup.hpp>

nex::ShadowMap::DepthPass::DepthPass() : TransformShader(ShaderProgram::create("shadow/shadow_map_depth_vs.glsl", "shadow/shadow_map_depth_fs.glsl"))
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
	data.internalFormat = InternalFormat::DEPTH32;
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

nex::TransformShader* nex::ShadowMap::getDepthPass()
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

void nex::ShadowMap::render(const nex::RenderCommandQueue::Buffer& shadowCommands, const RenderContext& context)
{
	
	RenderBackend::get()->setViewPort(0, 0, mRenderTarget->getWidth(), mRenderTarget->getHeight());
	mRenderTarget->bind();
	auto* depthBuffer = RenderBackend::get()->getDepthBuffer();
	depthBuffer->enableDepthBufferWriting(true);
	depthBuffer->enableDepthTest(true);
	mRenderTarget->clear(RenderComponent::Depth | RenderComponent::Color | RenderComponent::Stencil);

	PerspectiveCamera camera(800, 600);
	camera.setProjection(mProjection);
	camera.setView(mView, true);
	camera.setPrevViewProj(mProjection * mView);
	camera.setViewProj(mProjection * mView);

	RenderContext myContext = context;
	myContext.camera = &camera;


	mDepthPass->bind();
	mDepthPass->updateConstants(myContext);

	for (const auto& command : shadowCommands)
	{
		mDepthPass->uploadTransformMatrices(myContext, command);

		for (const auto& pair : command.batch->getEntries()) {
			Drawer::draw(mDepthPass.get(), pair.first, nullptr);
		}
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
	const auto look = normalize(glm::vec3(dirLight.directionWorld));
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
	mModel(model), mTextureView({}, { 256, 256 })
{
}

void nex::ShadowMap_ConfigurationView::drawSelf()
{
	auto* texture = mModel->getRenderResult();
	auto& imageDesc = mTextureView.getTextureDesc();
	imageDesc.texture = texture;
	imageDesc.flipY = ImageFactory::isYFlipped();
	imageDesc.sampler = nullptr;

	mTextureView.drawGUI();

	nex::gui::Separator(2.0f);
}