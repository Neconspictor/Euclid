#include <nex/opengl/shader/DepthMapShaderGL.hpp>
#include <glm/glm.hpp>

using namespace glm;
using namespace std;
using namespace nex;

CubeDepthMapShader::CubeDepthMapShader()
{
	mProgram = ShaderProgram::create(
		"depth_map_cube_vs.glsl", "depth_map_cube_fs.glsl");

	mModel = { mProgram->getUniformLocation("model"), UniformType::MAT4};
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mLightPos = { mProgram->getUniformLocation("lightPos"), UniformType::VEC3 };
	mRange = { mProgram->getUniformLocation("range"), UniformType::FLOAT };
	mCubeMap = { mProgram->getUniformLocation("cubeDepthMap"), UniformType::CUBE_MAP, 0};
}

void CubeDepthMapShader::useCubeDepthMap(const CubeMap* map)
{
	mProgram->setTexture(mCubeMap.location, map, mCubeMap.bindingSlot);
}

void CubeDepthMapShader::setLightPos(const vec3& pos)
{
	mProgram->setVec3(mLightPos.location, pos);
}

void CubeDepthMapShader::setRange(float range)
{
	mProgram->setFloat(mRange.location, range);
}

void CubeDepthMapShader::setModelMatrix(const glm::mat4& model)
{
	mProgram->setMat4(mModel.location, model);
}

void CubeDepthMapShader::setMVP(const glm::mat4& trafo)
{
	mProgram->setMat4(mTransform.location, trafo);
}

DepthMapShader::DepthMapShader()
{
	mProgram = ShaderProgram::create(
		"depth_map_vs.glsl", "depth_map_fs.glsl");

	mDephTexture = { mProgram->getUniformLocation("depthMap"), UniformType::TEXTURE2D, 0};
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

void DepthMapShader::useDepthMapTexture(const Texture* texture)
{
	mProgram->setTexture(mDephTexture.location, texture, mDephTexture.bindingSlot);
}

void DepthMapShader::setMVP(const glm::mat4& trafo)
{
	mProgram->setMat4(mTransform.location, trafo);
}

VarianceDepthMapShader::VarianceDepthMapShader()
{
	mProgram = ShaderProgram::create (
		"variance_depth_map_vs.glsl", "variance_depth_map_fs.glsl");

	mDephTexture = { mProgram->getUniformLocation("vDepthMap"), UniformType::TEXTURE2D, 0 };
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

void VarianceDepthMapShader::setMVP(const glm::mat4& trafo)
{
	mProgram->setMat4(mTransform.location, trafo);
}


void VarianceDepthMapShader::useVDepthMapTexture(const Texture* texture)
{
	mProgram->setTexture(mDephTexture.location, texture, mDephTexture.bindingSlot);
}