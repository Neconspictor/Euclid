#include <shader/opengl/DepthMapShaderGL.hpp>
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

CubeDepthMapShaderGL::CubeDepthMapShaderGL() :
	CubeDepthMapShader(), cubeMap(nullptr), range(0)
{
	using types = ShaderAttributeType;
	attributes.create(types::MAT4, nullptr, "transform");
	attributes.create(types::MAT4, nullptr, "model");
	attributes.create(types::VEC3, &lightPos, "lightPos", true);
	attributes.create(types::FLOAT, &range, "range", true);
	attributes.create(types::CubeMap, nullptr, "cubeDepthMap");
}

const ShaderAttribute* CubeDepthMapShaderGL::getAttributeList() const
{
	return attributes.getList();
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

	// static allocation for faster access
	static string cubeDepthMapName = "cubeDepthMap";
	attributes.setData(cubeDepthMapName, cubeMap);
}

void CubeDepthMapShaderGL::setLightPos(vec3 pos)
{
	lightPos = move(pos);

	// static allocation for faster access
	static string lightPosName = "lightPos";
	attributes.setData(lightPosName, &lightPos);
}

void CubeDepthMapShaderGL::setRange(float range)
{
	this->range = range;

	// static allocation for faster access
	static string rangeName = "range";
	attributes.setData(rangeName, &this->range);
}

void CubeDepthMapShaderGL::update(const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
	this->model = model;

	// static allocation for faster access
	static string transformName = "transform";
	static string modelName = "model";

	attributes.setData(transformName, &transform);
	attributes.setData(modelName, &this->model);
}

DepthMapShaderGL::DepthMapShaderGL() :
	DepthMapShader(), texture(nullptr)
{
	attributes.create(ShaderAttributeType::MAT4, nullptr, "transform");
	attributes.create(ShaderAttributeType::TEXTURE2D, nullptr, "depthMap");
}

DepthMapShaderGL::~DepthMapShaderGL(){}

void DepthMapShaderGL::useDepthMapTexture(Texture* texture)
{
	this->texture = dynamic_cast<TextureGL*>(texture);
	assert(this->texture != nullptr);

	static string depthMapName = "depthMap";
	attributes.setData(depthMapName, &this->texture);
}