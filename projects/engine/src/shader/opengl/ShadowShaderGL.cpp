#include <shader/opengl/ShadowShaderGL.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace std;
using namespace glm;

PointShadowShaderGL::PointShadowShaderGL() : PointShadowShader(), ShaderConfigGL(),
matrices(nullptr), range(0)
{
	attributes.create(ShaderAttributeType::MAT4, nullptr, "model");

	for (int i = 0; i < 6; ++i)
	{
		string matrixDesc = "shadowMatrices[" + to_string(i) + "]";
		attributes.create(ShaderAttributeType::MAT4, nullptr, matrixDesc);
	}

	attributes.create(ShaderAttributeType::FLOAT, &range, "range", true);
	attributes.create(ShaderAttributeType::VEC3, &lightPos, "lightPos", true);
}

PointShadowShaderGL::~PointShadowShaderGL(){}

void PointShadowShaderGL::setLightPosition(vec3 pos)
{
	this->lightPos = move(pos);
}

void PointShadowShaderGL::setRange(float range)
{
	this->range = range;
}

void PointShadowShaderGL::setShadowMatrices(mat4 matrices[6])
{
	this->matrices = matrices;
	for (int i = 0; i < 6; ++i)
	{
		string matrixDesc = "shadowMatrices[" + to_string(i) + "]";
		attributes.setData(matrixDesc, &this->matrices[i]);
	}
}

void PointShadowShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	attributes.setData("model", data.model);
}

ShadowShaderGL::ShadowShaderGL() : ShadowShader(), ShaderConfigGL()
{
	attributes.create(ShaderAttributeType::MAT4, &lightSpaceMatrix, "lightSpaceMatrix", true);
	attributes.create(ShaderAttributeType::MAT4, nullptr, "model");
}

ShadowShaderGL::~ShadowShaderGL(){}

void ShadowShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	lightSpaceMatrix = (*data.projection) * (*data.view);
	attributes.setData("model", data.model);
}

VarianceShadowShaderGL::VarianceShadowShaderGL() : VarianceShadowShader(), ShaderConfigGL()
{
	attributes.create(ShaderAttributeType::MAT4, &lightSpaceMatrix, "lightSpaceMatrix", true);
	attributes.create(ShaderAttributeType::MAT4, nullptr, "model");
}

VarianceShadowShaderGL::~VarianceShadowShaderGL() {}

void VarianceShadowShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	lightSpaceMatrix = (*data.projection) * (*data.view);
	attributes.setData("model", data.model);
}