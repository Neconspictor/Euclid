#include <shader/opengl/NormalsShaderGL.hpp>

using namespace std;
using namespace glm;

NormalsShaderGL::NormalsShaderGL() : color(0,0,0,1)
{
	using type = ShaderAttributeType;
	attributes.create(type::MAT4, &transform, "transform");
	attributes.create(type::MAT3, &normalMatrix, "normalMatrix");
	attributes.create(type::VEC4, &color, "color");
}

NormalsShaderGL::~NormalsShaderGL() {}

const ShaderAttribute* NormalsShaderGL::getAttributeList() const
{
	return attributes.getList();
}

const vec4& NormalsShaderGL::getNormalColor() const
{
	return color;
}

int NormalsShaderGL::getNumberOfAttributes() const
{
	return attributes.size();
}

void NormalsShaderGL::setNormalColor(vec4 color)
{
	this->color = move(color);

	static string colorName = "color";
	attributes.setData(colorName, &this->color);
}

void NormalsShaderGL::update(const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
	normalMatrix = transpose(inverse(view * model));

	// static allocation for faster access
	static string transformName = "transform";
	static string normalMatrixName = "normalMatrix";
	static string projectionName = "projection";

	attributes.setData(transformName, &transform);
	attributes.setData(normalMatrixName, &normalMatrix);
	attributes.setData(projectionName, (void*)data.projection);
}