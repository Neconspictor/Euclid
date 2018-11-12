#include <nex/opengl/shader/ShadowShaderGL.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>

using namespace std;
using namespace glm;

PointShadowShaderGL::PointShadowShaderGL() : ShaderConfigGL(),
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

ShadowShaderGL::ShadowShaderGL() : ShaderConfigGL()
{
	attributes.create(ShaderAttributeType::MAT4, &lightSpaceMatrix, "lightSpaceMatrix", true);
	attributes.create(ShaderAttributeType::MAT4, nullptr, "model");
}

ShadowShaderGL::~ShadowShaderGL(){}

void ShadowShaderGL::beforeDrawing(const MeshGL& mesh)
{
	/*
	glBindVertexArray(mesh.getVertexArrayObject());
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
	glBindVertexArray(0);

	*/
}

void ShadowShaderGL::afterDrawing(const MeshGL& mesh)
{
	/*
	glBindVertexArray(mesh.getVertexArrayObject());
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
	glBindVertexArray(0);
	*/
}

void ShadowShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	lightSpaceMatrix = (*data.projection) * (*data.view);
	attributes.setData("model", data.model);
}

VarianceShadowShaderGL::VarianceShadowShaderGL() : ShaderConfigGL()
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