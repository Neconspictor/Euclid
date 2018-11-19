#include <nex/opengl/shader/DepthMapShaderGL.hpp>
#include <glm/glm.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>

using namespace glm;
using namespace std;

CubeDepthMapShaderGL::CubeDepthMapShaderGL()
{
	mProgram = new ShaderProgramGL(
		"depth_map_cube_vs.glsl", "depth_map_cube_fs.glsl");

	mModel = { mProgram->getUniformLocation("model"), UniformType::MAT4};
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mLightPos = { mProgram->getUniformLocation("lightPos"), UniformType::VEC3 };
	mRange = { mProgram->getUniformLocation("range"), UniformType::FLOAT };
	mCubeMap = { mProgram->getUniformLocation("cubeDepthMap"), UniformType::CUBE_MAP, 0};
}

CubeDepthMapShaderGL::~CubeDepthMapShaderGL(){}

void CubeDepthMapShaderGL::useCubeDepthMap(const CubeMapGL* map)
{
	mProgram->setTexture(mCubeMap.location, map, mCubeMap.textureUnit);
}

void CubeDepthMapShaderGL::setLightPos(const vec3& pos)
{
	mProgram->setVec3(mLightPos.location, pos);
}

void CubeDepthMapShaderGL::setRange(float range)
{
	mProgram->setFloat(mRange.location, range);
}

void CubeDepthMapShaderGL::setModelMatrix(const glm::mat4& model)
{
	mProgram->setMat4(mModel.location, model);
}

void CubeDepthMapShaderGL::setMVP(const glm::mat4& trafo)
{
	mProgram->setMat4(mTransform.location, trafo);
}

DepthMapShaderGL::DepthMapShaderGL()
{
	mProgram = new ShaderProgramGL(
		"depth_map_vs.glsl", "depth_map_fs.glsl");

	mDephTexture = { mProgram->getUniformLocation("depthMap"), UniformType::TEXTURE2D, 0};
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

void DepthMapShaderGL::useDepthMapTexture(const TextureGL* texture)
{
	mProgram->setTexture(mDephTexture.location, texture, mDephTexture.textureUnit);
}

void DepthMapShaderGL::setMVP(const glm::mat4& trafo)
{
	mProgram->setMat4(mTransform.location, trafo);
}

VarianceDepthMapShaderGL::VarianceDepthMapShaderGL()
{
	mProgram = new ShaderProgramGL(
		"variance_depth_map_vs.glsl", "variance_depth_map_fs.glsl");

	mDephTexture = { mProgram->getUniformLocation("vDepthMap"), UniformType::TEXTURE2D, 0 };
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

void VarianceDepthMapShaderGL::setMVP(const glm::mat4& trafo)
{
	mProgram->setMat4(mTransform.location, trafo);
}


void VarianceDepthMapShaderGL::useVDepthMapTexture(const TextureGL* texture)
{
	mProgram->setTexture(mDephTexture.location, texture, mDephTexture.textureUnit);
}