#include <nex/post_processing/TAA.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/material/Material.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/drawing/MeshDrawer.hpp>
#include <nex/camera/Camera.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/texture/RenderTarget.hpp>


class nex::TAA::TaaPass : public Shader 
{
public:
	TaaPass(bool useGamma) : Shader()
	{
		std::vector<std::string> defines;
		if (useGamma) {
			//defines.push_back("#define SOURCE_GAMMA_SPACE\n");
		}

		defines.push_back("#define MINMAX_3X3 0\n");
		defines.push_back("#define MINMAX_3X3_ROUNDED 1\n");
		defines.push_back("#define MINMAX_4TAP_VARYING 0\n");
		defines.push_back("#define USE_YCOCG 0\n");
		defines.push_back("#define USE_CLIPPING 1\n");
		defines.push_back("#define UNJITTER_COLORSAMPLES 0\n");
		defines.push_back("#define UNJITTER_NEIGHBORHOOD 0\n");
		defines.push_back("#define UNJITTER_REPROJECTION 0\n");
		defines.push_back("#define USE_DILATION 1\n");
		

		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/taa_fs.glsl", 
			nullptr, nullptr, nullptr, defines);

		mInverseCurrentViewProjection = { mProgram->getUniformLocation("inverseViewProjectionCURRENT"), UniformType::MAT4 };
		mViewProjectionHistory = { mProgram->getUniformLocation("viewProjectionHISTORY"), UniformType::MAT4 };

		mSource = mProgram->createTextureUniform("colourRENDER", UniformType::TEXTURE2D, 0);
		mVelocity = mProgram->createTextureUniform("velocityBUF", UniformType::TEXTURE2D, 1);
		mDepth = mProgram->createTextureUniform("depthRENDER", UniformType::TEXTURE2D, 2);
		mSourceHistory = mProgram->createTextureUniform("colourANTIALIASED", UniformType::TEXTURE2D, 3);
		mTextureSize = { mProgram->getUniformLocation("windowSize"), UniformType::VEC2 };
		mPixelSize = { mProgram->getUniformLocation("pixelSize"), UniformType::VEC2 };
		mJitter = { mProgram->getUniformLocation("jitter"), UniformType::VEC2 };
		mJitterPrev = { mProgram->getUniformLocation("jitterHISTORY"), UniformType::VEC2 };
		mFeedback = { mProgram->getUniformLocation("feedback"), UniformType::FLOAT };
		mFeedbackMin = { mProgram->getUniformLocation("_FeedbackMin"), UniformType::FLOAT };
		mFeedbackMax = { mProgram->getUniformLocation("_FeedbackMax"), UniformType::FLOAT };
		mClipInfo = { mProgram->getUniformLocation("clipInfo"), UniformType::VEC4 };
	}

	const RenderState& getState() const {
		return RenderState::getNoDepthTest();
	}

	void setInverseCurrentViewProjection(const glm::mat4& mat) 
	{
		mProgram->setMat4(mInverseCurrentViewProjection.location, mat);
	}

	void setViewProjectionHistory(const glm::mat4& mat)
	{
		mProgram->setMat4(mViewProjectionHistory.location, mat);
	}

	void setSource(Texture* source) {
		mProgram->setTexture(source, Sampler::getLinear(), mSource.bindingSlot);
	}

	void setVelocity(Texture* source) {
		mProgram->setTexture(source, Sampler::getLinear(), mVelocity.bindingSlot);
	}

	void setDepth(Texture* source) {
		mProgram->setTexture(source, Sampler::getLinear(), mDepth.bindingSlot);
	}

	void setSourceHistory(Texture* source) {
		mProgram->setTexture(source, Sampler::getLinear(), mSourceHistory.bindingSlot);
	}

	void setTextureSize(const glm::vec2& vec)
	{
		mProgram->setVec2(mTextureSize.location, vec);
	}

	void setPixelSize(const glm::vec2& vec)
	{
		mProgram->setVec2(mPixelSize.location, vec);
	}

	void setJitter(const glm::vec2& vec)
	{
		mProgram->setVec2(mJitter.location, vec);
	}

	void setJitterPrev(const glm::vec2& vec)
	{
		mProgram->setVec2(mJitterPrev.location, vec);
	}

	void setFeedBack(float value)
	{
		value = std::clamp<float>(value, 0.0f, 1.0f);
		mProgram->setFloat(mFeedback.location, value);
	}

	void setFeedBackMin(float value)
	{
		value = std::clamp<float>(value, 0.0f, 1.0f);
		mProgram->setFloat(mFeedbackMin.location, value);
	}

	void setFeedBackMax(float value)
	{
		value = std::clamp<float>(value, 0.0f, 1.0f);
		mProgram->setFloat(mFeedbackMax.location, value);
	}

	void setClipInfo(const glm::vec4& info)
	{
		mProgram->setVec4(mClipInfo.location, info);
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
	Uniform mJitterPrev;
	Uniform mFeedback;
	Uniform mFeedbackMin;
	Uniform mFeedbackMax;
	Uniform mClipInfo;

	RenderState mState;
};

nex::TAA::TAA() :
mTaaPass(std::make_unique<TaaPass>(true)),
mJitterCursor(0),
mJitterCursorPrev(mJitterVector.size()-1),
mFeedback(0.25f),
mJitterMatrix(glm::mat4(1.0f))
{
	//updateJitterVectors(glm::vec2(1.0f));
	//updateJitterMatrix();
}

nex::TAA::~TAA() = default;

void nex::TAA::advanceJitter()
{
	advanceJitterCursor();
	updateJitterMatrix();
}

void nex::TAA::advanceJitterCursor()
{
	mJitterCursorPrev = mJitterCursor;
	mJitterCursor = (mJitterCursor + 1) % points_Halton_2_3_x16.size();
}

float nex::TAA::haltonSeq(int prime, int index)
{
	float r = 0.0f;
	float f = 1.0f;
	int i = index;
	while (i > 0)
	{
		f /= prime;
		r += f * (i % prime);
		i = (int)std::floorf(i / (float)prime);
	}
	return r;
}

void nex::TAA::initializeHalton_2_3(glm::vec2* vecs, size_t size)
{
	for (auto i = 0; i < size; ++i)
	{
		float u = haltonSeq(2, i + 1) - 0.5f;
		float v = haltonSeq(3, i + 1) - 0.5f;
		vecs[i] = { u, v };
	}
}

void nex::TAA::updateJitterMatrix()
{
	auto translation = glm::vec3(mJitterVector[mJitterCursor], 0.0f);
	mJitterMatrix = glm::translate(glm::mat4(1.0f), translation); //glm::mat4(1.0f)
}

void nex::TAA::antialias(Texture* source, Texture* sourceHistory, Texture* depth, Texture* velocity, const Camera& camera)
{
	mTaaPass->bind();
	glm::vec2 textureSize(source->getWidth(), source->getHeight());
	glm::vec2 inverseSize = glm::vec2(1.0f) / textureSize;
	mTaaPass->setInverseCurrentViewProjection(inverse(camera.getViewProj()));
	mTaaPass->setViewProjectionHistory(camera.getViewProjPrev());
	mTaaPass->setSource(source);
	mTaaPass->setDepth(depth);
	mTaaPass->setSourceHistory(sourceHistory);
	mTaaPass->setVelocity(velocity);
	mTaaPass->setTextureSize(textureSize);
	mTaaPass->setPixelSize(inverseSize);
	//mTaaPass->setJitter(glm::vec2(0.00f)); // We ignore the jitter for now TODO
	mTaaPass->setJitter(mJitterVector[mJitterCursor]);
	//mTaaPass->setJitterPrev(mJitterVector[mJitterCursorPrev]);
	mTaaPass->setFeedBack(mFeedback);
	mTaaPass->setFeedBackMin(0.25f);
	mTaaPass->setFeedBackMax(1.0f);
	mTaaPass->setClipInfo(camera.getClipInfo());

	MeshDrawer::drawFullscreenTriangle(mTaaPass->getState(), mTaaPass.get());
}

void nex::TAA::updateJitterVectors(const glm::vec2& pixelSizeScreenSpace)
{
	initializeHalton_2_3(points_Halton_2_3_x16.data(), points_Halton_2_3_x16.size());

	for (size_t i = 0; i < mJitterVector.size(); i++) {
		mJitterVector[i] = points_Halton_2_3_x16[i] * pixelSizeScreenSpace;
	}

	//updateJitterMatrix();
}

const glm::mat4& nex::TAA::getJitterMatrix() const
{
	return mJitterMatrix;
}

const glm::vec2& nex::TAA::getJitterVec() const
{
	return points_Halton_2_3_x16[mJitterCursor];
}

float nex::TAA::getFeedback() const
{
	return mFeedback;
}

void nex::TAA::setFeedback(float value)
{
	mFeedback = value;
}