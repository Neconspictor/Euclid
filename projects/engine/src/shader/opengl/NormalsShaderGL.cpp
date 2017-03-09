#include <shader/opengl/NormalsShaderGL.hpp>

using namespace std;
using namespace glm;

NormalsShaderGL::NormalsShaderGL() : color(0,0,0,1)
{
	using type = ShaderAttributeType;
	attributes.create(type::MAT4, nullptr, "projection");
	attributes.create(type::MAT4, &transform, "transform", true);
	attributes.create(type::MAT3, &normalMatrix, "normalMatrix", true);
	attributes.create(type::VEC4, &color, "color", true);
}

NormalsShaderGL::~NormalsShaderGL() {}

const vec4& NormalsShaderGL::getNormalColor() const
{
	return color;
}

void NormalsShaderGL::setNormalColor(vec4 color)
{
	this->color = move(color);

	static string colorName = "color";
}

void NormalsShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
	normalMatrix = transpose(inverse(view * model));

	// static allocation for faster access
	static string projectionName = "projection";

	attributes.setData(projectionName, data.projection);
}