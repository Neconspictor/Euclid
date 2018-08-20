#include <nex/opengl/shader/SimpleColorShaderGL.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>

using namespace std;
using namespace glm;

SimpleColorShaderGL::SimpleColorShaderGL() : SimpleColorShader(), ShaderConfigGL(), 
objectColor(1, 1, 1, 1)
{
	attributes.create(ShaderAttributeType::VEC4, &objectColor, "objectColor", true);
	attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
}

SimpleColorShaderGL::~SimpleColorShaderGL(){}

const vec4& SimpleColorShaderGL::getObjectColor() const
{
	return objectColor;
}

void SimpleColorShaderGL::setObjectColor(vec4 color)
{
	objectColor = move(color);
}

void SimpleColorShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	transform = projection * view * model;
}