#include <nex/shader/ShadowShader.hpp>
#include <nex/mesh/SubMesh.hpp>

using namespace std;
using namespace glm;
using namespace nex;

PointShadowPass::PointShadowPass()
{
	mShader = Shader::create("shadow_point_vs.glsl", "shadow_point_fs.glsl", "shadow_point_gs.glsl");

	mModel = { mShader->getUniformLocation("model"), UniformType::MAT4 };

	for (int i = 0; i < 6; ++i)
	{
		string matrixDesc = "shadowMatrices[" + to_string(i) + "]";
		mShadowMatrices[i] = { mShader->getUniformLocation(matrixDesc.c_str()), UniformType::MAT4};
	}

	mRange = { mShader->getUniformLocation("range"), UniformType::FLOAT };
	mLightPos = { mShader->getUniformLocation("lightPos"), UniformType::VEC3 };
}

void PointShadowPass::setLightPosition(const glm::vec3& pos)
{
	mShader->setVec3(mLightPos.location, pos);
}

void PointShadowPass::setRange(float range)
{
	mShader->setFloat(mRange.location, range);
}

void PointShadowPass::setShadowMatrices(const glm::mat4* matrices)
{
	for (unsigned i = 0; i < 6; ++i)
	{
		mShader->setMat4(mShadowMatrices[i].location, matrices[i]);
	}
}

void PointShadowPass::setModel(const glm::mat4& mat)
{
	mShader->setMat4(mModel.location, mat);
}

ShadowPass::ShadowPass()
{
	mShader = Shader::create("shadow_vs.glsl", "shadow_fs.glsl");

	mModel = { mShader->getUniformLocation("model"), UniformType::MAT4 };
	mLightSpaceMatrix = { mShader->getUniformLocation("lightSpaceMatrix"), UniformType::MAT4 };
}

void ShadowPass::setModel(const glm::mat4& mat)
{
	mShader->setMat4(mModel.location, mat);
}

void ShadowPass::setLightSpaceMatrix(const glm::mat4& mat)
{
	mShader->setMat4(mLightSpaceMatrix.location, mat);
}

void ShadowPass::onModelMatrixUpdate(const glm::mat4& modelMatrix)
{
	setModel(modelMatrix);
}


//TODO
/*
void ShadowShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	lightSpaceMatrix = (*data.projection) * (*data.view);
	attributes.setData("model", data.model);
}*/

VarianceShadowPass::VarianceShadowPass()
{
	mShader = Shader::create("variance_shadow_vs.glsl", "variance_shadow_fs.glsl");

	mModel = { mShader->getUniformLocation("model"), UniformType::MAT4 };
	mLightSpaceMatrix = { mShader->getUniformLocation("lightSpaceMatrix"), UniformType::MAT4 };
}

void VarianceShadowPass::setModel(const glm::mat4& mat)
{
	mShader->setMat4(mModel.location, mat);
}

void VarianceShadowPass::setLightSpaceMatrix(const glm::mat4& mat)
{
	mShader->setMat4(mLightSpaceMatrix.location, mat);
}