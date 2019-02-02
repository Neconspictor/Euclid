#include <nex/shader/ShadowShader.hpp>
#include <nex/mesh/SubMesh.hpp>

using namespace std;
using namespace glm;
using namespace nex;

PointShadowShader::PointShadowShader()
{
	mProgram = ShaderProgram::create("shadow_point_vs.glsl", "shadow_point_fs.glsl", "shadow_point_gs.glsl");

	mModel = { mProgram->getUniformLocation("model"), UniformType::MAT4 };

	for (int i = 0; i < 6; ++i)
	{
		string matrixDesc = "shadowMatrices[" + to_string(i) + "]";
		mShadowMatrices[i] = { mProgram->getUniformLocation(matrixDesc.c_str()), UniformType::MAT4};
	}

	mRange = { mProgram->getUniformLocation("range"), UniformType::FLOAT };
	mLightPos = { mProgram->getUniformLocation("lightPos"), UniformType::VEC3 };
}

void PointShadowShader::setLightPosition(const glm::vec3& pos)
{
	mProgram->setVec3(mLightPos.location, pos);
}

void PointShadowShader::setRange(float range)
{
	mProgram->setFloat(mRange.location, range);
}

void PointShadowShader::setShadowMatrices(const glm::mat4* matrices)
{
	for (unsigned i = 0; i < 6; ++i)
	{
		mProgram->setMat4(mShadowMatrices[i].location, matrices[i]);
	}
}

void PointShadowShader::setModel(const glm::mat4& mat)
{
	mProgram->setMat4(mModel.location, mat);
}

ShadowShader::ShadowShader()
{
	mProgram = ShaderProgram::create("shadow_vs.glsl", "shadow_fs.glsl");

	mModel = { mProgram->getUniformLocation("model"), UniformType::MAT4 };
	mLightSpaceMatrix = { mProgram->getUniformLocation("lightSpaceMatrix"), UniformType::MAT4 };
}

void ShadowShader::setModel(const glm::mat4& mat)
{
	mProgram->setMat4(mModel.location, mat);
}

void ShadowShader::setLightSpaceMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mLightSpaceMatrix.location, mat);
}

void ShadowShader::onModelMatrixUpdate(const glm::mat4& modelMatrix)
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

VarianceShadowShader::VarianceShadowShader()
{
	mProgram = ShaderProgram::create("variance_shadow_vs.glsl", "variance_shadow_fs.glsl");

	mModel = { mProgram->getUniformLocation("model"), UniformType::MAT4 };
	mLightSpaceMatrix = { mProgram->getUniformLocation("lightSpaceMatrix"), UniformType::MAT4 };
}

void VarianceShadowShader::setModel(const glm::mat4& mat)
{
	mProgram->setMat4(mModel.location, mat);
}

void VarianceShadowShader::setLightSpaceMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mLightSpaceMatrix.location, mat);
}