#include <nex/opengl/shader/SimpleExtrudeShaderGL.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>

using namespace std;
using namespace glm;

SimpleExtrudeShaderGL::SimpleExtrudeShaderGL() : SimpleExtrudeShader(), ShaderConfigGL(), 
objectColor(1, 1, 1, 1), extrudeValue(0)
{
	attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
	attributes.create(ShaderAttributeType::VEC4, &objectColor, "objectColor", true);
	attributes.create(ShaderAttributeType::FLOAT, &extrudeValue, "extrudeValue", true);
}

SimpleExtrudeShaderGL::~SimpleExtrudeShaderGL(){}

const vec4& SimpleExtrudeShaderGL::getObjectColor() const
{
	return objectColor;
}

void SimpleExtrudeShaderGL::setExtrudeValue(float extrudeValue)
{
	this->extrudeValue = extrudeValue;
}

void SimpleExtrudeShaderGL::setObjectColor(vec4 color)
{
	objectColor = move(color);
}

void SimpleExtrudeShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	transform = projection * view * model;
}