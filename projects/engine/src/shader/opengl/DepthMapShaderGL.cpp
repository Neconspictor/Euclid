#include <shader/opengl/DepthMapShaderGL.hpp>
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

CubeDepthMapShaderGL::CubeDepthMapShaderGL() :
	CubeDepthMapShader(), cubeMap(nullptr), range(0)
{
	ShaderAttributeGL attribute = { ShaderAttributeType::MAT4X4, nullptr, "transform" };
	attributes.push_back(attribute);
	attribute = { ShaderAttributeType::MAT4X4, nullptr, "model" };
	attributes.push_back(attribute);
	attribute = { ShaderAttributeType::VEC3, &lightPos, "lightPos", true };
	attributes.push_back(attribute);
	attribute = { ShaderAttributeType::FLOAT, &range, "range", true };
	attributes.push_back(attribute);
	attribute = { ShaderAttributeType::CubeMap, nullptr, "cubeDepthMap" };
	attributes.push_back(attribute);
}

const ShaderAttribute* CubeDepthMapShaderGL::getAttributeList() const
{
	if (attributes.size() == 0) return nullptr;
	return attributes.data();
}

int CubeDepthMapShaderGL::getNumberOfAttributes() const
{
	return attributes.size();
}

CubeDepthMapShaderGL::~CubeDepthMapShaderGL(){}

void CubeDepthMapShaderGL::useCubeDepthMap(CubeMap* map)
{
	this->cubeMap = dynamic_cast<CubeMapGL*>(map);
	assert(this->cubeMap != nullptr);
	auto attr = ShaderAttributeGL::search(attributes, "cubeDepthMap");
	attr->setData(cubeMap);
	attr->activate(true);
}

void CubeDepthMapShaderGL::setLightPos(vec3 pos)
{
	lightPos = move(pos);
	auto attr = ShaderAttributeGL::search(attributes, "lightPos");
	attr->setData(&lightPos);
	attr->activate(true);
}

void CubeDepthMapShaderGL::setRange(float range)
{
	this->range = range;
	auto attr = ShaderAttributeGL::search(attributes, "range");
	attr->setData(&range);
	attr->activate(true);
}

void CubeDepthMapShaderGL::update(const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
	this->model = model;

	auto attr = ShaderAttributeGL::search(attributes, "transform");
	attr->setData(&transform);
	attr->activate(true);

	attr = ShaderAttributeGL::search(attributes, "model");
	attr->setData(&this->model);
	attr->activate(true);
}

DepthMapShaderGL::DepthMapShaderGL() :
	DepthMapShader(), texture(nullptr)
{
	ShaderAttributeGL attribute = { ShaderAttributeType::MAT4X4, nullptr, "transform" };
	attributes.push_back(attribute);
	attribute = { ShaderAttributeType::TEXTURE2D, nullptr, "depthMap" };
	attributes.push_back(attribute);
}

DepthMapShaderGL::~DepthMapShaderGL(){}

void DepthMapShaderGL::useDepthMapTexture(Texture* texture)
{
	this->texture = dynamic_cast<TextureGL*>(texture);
	assert(this->texture != nullptr);
}