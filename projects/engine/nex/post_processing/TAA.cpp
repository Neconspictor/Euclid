#include <nex/post_processing/TAA.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/material/Material.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/camera/Camera.hpp>

#include <glm/gtc/matrix_transform.hpp>


class nex::TAA::TaaPass : public Pass 
{
public:
	TaaPass(bool useGamma) : Pass()
	{
		std::vector<std::string> defines;
		if (useGamma) {
			defines.push_back("#define SOURCE_GAMMA_SPACE");
		}

		mShader = Shader::create("fullscreenPlane_vs.glsl", "post_processing/taa_fs.glsl", 
			nullptr, nullptr, nullptr, defines);

		mInverseCurrentViewProjection = { mShader->getUniformLocation("inverseViewProjectionCURRENT"), UniformType::MAT4 };
		mViewProjectionHistory = { mShader->getUniformLocation("viewProjectionHISTORY"), UniformType::MAT4 };

		mSource = mShader->createTextureUniform("colourRENDER", UniformType::TEXTURE2D, 0);
		mVelocity = mShader->createTextureUniform("velocityBUF", UniformType::TEXTURE2D, 1);
		mDepth = mShader->createTextureUniform("depthRENDER", UniformType::TEXTURE2D, 2);
		mSourceHistory = mShader->createTextureUniform("colourANTIALIASED", UniformType::TEXTURE2D, 3);
		mTextureSize = { mShader->getUniformLocation("windowSize"), UniformType::VEC2 };
		mPixelSize = { mShader->getUniformLocation("pixelSize"), UniformType::VEC2 };
		mJitter = { mShader->getUniformLocation("jitter"), UniformType::VEC2 };
		mFeedback = { mShader->getUniformLocation("feedback"), UniformType::FLOAT };

		mState = RenderState::createNoDepthTest();

		mSampler.setMinFilter(TextureFilter::NearestNeighbor);
		mSampler.setMagFilter(TextureFilter::NearestNeighbor);
	}

	const RenderState& getState() const {
		return mState;
	}

	void setInverseCurrentViewProjection(const glm::mat4& mat) 
	{
		mShader->setMat4(mInverseCurrentViewProjection.location, mat);
	}

	void setViewProjectionHistory(const glm::mat4& mat)
	{
		mShader->setMat4(mViewProjectionHistory.location, mat);
	}

	void setSource(Texture* source) {
		mShader->setTexture(source, &mSampler, mSource.bindingSlot);
	}

	void setVelocity(Texture* source) {
		mShader->setTexture(source, &mSampler, mVelocity.bindingSlot);
	}

	void setDepth(Texture* source) {
		mShader->setTexture(source, &mSampler, mDepth.bindingSlot);
	}

	void setSourceHistory(Texture* source) {
		mShader->setTexture(source, &mSampler, mSourceHistory.bindingSlot);
	}

	void setTextureSize(const glm::vec2& vec)
	{
		mShader->setVec2(mTextureSize.location, vec);
	}

	void setPixelSize(const glm::vec2& vec)
	{
		mShader->setVec2(mPixelSize.location, vec);
	}

	void setJitter(const glm::vec2& vec)
	{
		mShader->setVec2(mJitter.location, vec);
	}

	void setFeedBack(float value)
	{
		value = std::clamp<float>(value, 0.0f, 1.0f);
		mShader->setFloat(mFeedback.location, value);
	}

private:

	Uniform mInverseFrameBufferSize;
	Uniform mInverseCurrentViewProjection;
	Uniform mViewProjectionHistory;
	UniformTex mSource;
	UniformTex mVelocity;
	UniformTex mDepth;
	UniformTex mSourceHistory;
	Uniform mTextureSize;
	Uniform mPixelSize;
	Uniform mJitter;
	Uniform mFeedback;

	RenderState mState;
};

nex::TAA::TAA() :
mTaaPass(std::make_unique<TaaPass>(true)),
mJitterCursor(0),
mFeedback(0.5f),
mJitterMatrix(glm::mat4(1.0f))
{
	updateJitterVectors(glm::vec2(1.0f));
	updateJitterMatrix();
}

nex::TAA::~TAA() = default;

void nex::TAA::advanceJitter()
{
	advanceJitterCursor();
	updateJitterMatrix();
}

void nex::TAA::advanceJitterCursor()
{
	mJitterCursor = (mJitterCursor + 1) % mJitterVector.size();
}

void nex::TAA::updateJitterMatrix()
{
	auto translation = glm::vec3(mJitterVector[mJitterCursor], 0.0f);
	mJitterMatrix = glm::translate(mJitterMatrix, translation);
}

void nex::TAA::antialias(Texture* source, Texture* sourceHistory, Texture* depth, const Camera& camera)
{
	mTaaPass->bind();
	glm::vec2 textureSize(source->getWidth(), source->getHeight());
	glm::vec2 inverseSize = glm::vec2(1.0f) / textureSize;
	auto invViewProj = inverse(camera.getProjectionMatrix() * camera.getView());
	mTaaPass->setInverseCurrentViewProjection(invViewProj);
	mTaaPass->setViewProjectionHistory(camera.getProjectionMatrix() * camera.getViewPrev());
	mTaaPass->setSource(source);
	mTaaPass->setDepth(depth);
	mTaaPass->setSourceHistory(sourceHistory);
	mTaaPass->setTextureSize(textureSize);
	mTaaPass->setPixelSize(inverseSize);
	mTaaPass->setJitter(glm::vec2(0.00f)); // We ignore the jitter for now TODO
	mTaaPass->setFeedBack(mFeedback);

	StaticMeshDrawer::drawFullscreenTriangle(mTaaPass->getState(), mTaaPass.get());
}

void nex::TAA::updateJitterVectors(const glm::vec2& pixelSizeScreenSpace)
{
	for (size_t i = 0; i < mJitterVector.size(); i++) {
		mJitterVector[i] = mSampleVector[i] * pixelSizeScreenSpace * 0.9f;
	}
}

const glm::mat4& nex::TAA::getJitterMatrix() const
{
	return mJitterMatrix;
}

float nex::TAA::getFeedback() const
{
	return mFeedback;
}

void nex::TAA::setFeedback(float value)
{
	mFeedback = value;
}