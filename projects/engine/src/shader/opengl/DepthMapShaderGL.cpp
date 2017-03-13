#include <shader/opengl/DepthMapShaderGL.hpp>
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

CubeDepthMapShaderGL::CubeDepthMapShaderGL() :
	CubeDepthMapShader(), cubeMap(nullptr), range(0)
{
	using types = ShaderAttributeType;
	attributes.create(types::MAT4, &transform, "transform", true);
	attributes.create(types::MAT4, nullptr, "model");
	attributes.create(types::VEC3, &lightPos, "lightPos", true);
	attributes.create(types::FLOAT, &range, "range", true);
	attributes.create(types::CUBE_MAP, nullptr, "cubeDepthMap");
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
}

void CubeDepthMapShaderGL::setRange(float range)
{
	this->range = range;

	// static allocation for faster access
	static string rangeName = "range";
}

void CubeDepthMapShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;

	// static allocation for faster access
	static string modelName = "model";
	attributes.setData(modelName, &model);
}

DepthMapShaderGL::DepthMapShaderGL() :
	DepthMapShader(), texture(nullptr)
{
	attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
	attributes.create(ShaderAttributeType::TEXTURE2D, nullptr, "depthMap");
}

DepthMapShaderGL::~DepthMapShaderGL(){}

void DepthMapShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	transform = (*data.projection) * (*data.view) * (*data.model);
}

void DepthMapShaderGL::useDepthMapTexture(Texture* texture)
{
	this->texture = dynamic_cast<TextureGL*>(texture);
	assert(this->texture != nullptr);

	static string depthMapName = "depthMap";
	attributes.setData(depthMapName, this->texture);
}