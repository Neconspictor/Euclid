#include <nex/shader/DepthMapPass.hpp>
#include <glm/glm.hpp>

using namespace glm;
using namespace std;
using namespace nex;

CubeDepthMapPass::CubeDepthMapPass()
{
	mShader = Shader::create(
		"depth_map_cube_vs.glsl", "depth_map_cube_fs.glsl");

	mModel = { mShader->getUniformLocation("model"), UniformType::MAT4};
	mTransform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	mLightPos = { mShader->getUniformLocation("lightPos"), UniformType::VEC3 };
	mRange = { mShader->getUniformLocation("range"), UniformType::FLOAT };
	mCubeMap = { mShader->getUniformLocation("cubeDepthMap"), UniformType::CUBE_MAP, 0};

	mShader->setBinding(mCubeMap.location, mCubeMap.bindingSlot);

	auto state = mSampler.getState();
	state.wrapR = state.wrapS = state.wrapT = TextureUVTechnique::ClampToBorder;
	state.borderColor = glm::vec4(1.0);
	mSampler.setState(state);
}

void CubeDepthMapPass::useCubeDepthMap(const CubeMap* map)
{
	mShader->setTexture(map, &mSampler, mCubeMap.bindingSlot);
}

void CubeDepthMapPass::setLightPos(const vec3& pos)
{
	mShader->setVec3(mLightPos.location, pos);
}

void CubeDepthMapPass::setRange(float range)
{
	mShader->setFloat(mRange.location, range);
}

void CubeDepthMapPass::setModelMatrix(const glm::mat4& model)
{
	mShader->setMat4(mModel.location, model);
}

void CubeDepthMapPass::setMVP(const glm::mat4& trafo)
{
	mShader->setMat4(mTransform.location, trafo);
}

DepthMapPass::DepthMapPass()
{
	mShader = Shader::create(
		"depth_map_vs.glsl", "depth_map_fs.glsl");

	mDephTexture = { mShader->getUniformLocation("depthMap"), UniformType::TEXTURE2D, 0};
	mTransform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };

	mShader->setBinding(mDephTexture.location, mDephTexture.bindingSlot);

	auto state = mSampler.getState();
	state.wrapR = state.wrapS = state.wrapT = TextureUVTechnique::ClampToBorder;
	state.borderColor = glm::vec4(1.0);
	mSampler.setState(state);
}

void DepthMapPass::onTransformUpdate(const TransformData& data)
{
	setMVP(*data.projection * *data.view * *data.model);
}

void DepthMapPass::useDepthMapTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mDephTexture.bindingSlot);
}

void DepthMapPass::setMVP(const glm::mat4& trafo)
{
	mShader->setMat4(mTransform.location, trafo);
}

VarianceDepthMapPass::VarianceDepthMapPass()
{
	mShader = Shader::create (
		"variance_depth_map_vs.glsl", "variance_depth_map_fs.glsl");

	mDephTexture = { mShader->getUniformLocation("vDepthMap"), UniformType::TEXTURE2D, 0 };
	mTransform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };

	mShader->setBinding(mDephTexture.location, mDephTexture.bindingSlot);

	auto state = mSampler.getState();
	state.wrapR = state.wrapS = state.wrapT = TextureUVTechnique::ClampToBorder;
	state.borderColor = glm::vec4(1.0);
	mSampler.setState(state);
}

void VarianceDepthMapPass::setMVP(const glm::mat4& trafo)
{
	mShader->setMat4(mTransform.location, trafo);
}


void VarianceDepthMapPass::useVDepthMapTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mDephTexture.bindingSlot);
}