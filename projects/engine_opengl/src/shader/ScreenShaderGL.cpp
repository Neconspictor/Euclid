#include <shader/ScreenShaderGL.hpp>
#include <glm/glm.hpp>
#include <mesh/MeshGL.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace std;
using namespace glm;

ScreenShaderGL::ScreenShaderGL() :
	ScreenShader(), ShaderConfigGL(), texture(nullptr)
{
	attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
	attributes.create(ShaderAttributeType::TEXTURE2D, nullptr, "screenTexture");
}

ScreenShaderGL::~ScreenShaderGL(){}

void ScreenShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
}

void ScreenShaderGL::useTexture(Texture* texture)
{
	this->texture = dynamic_cast<TextureGL*>(texture);
	assert(this->texture);
	attributes.setData("screenTexture", this->texture);
}