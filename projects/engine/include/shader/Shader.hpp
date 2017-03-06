#pragma once

#include <mesh/Mesh.hpp>
#include <glm/glm.hpp>
#include <list>
#include <platform/util/StringUtils.hpp>

/**
* Enumerates all shaders that can be used for shading models.
*/
enum class Shaders
{
	BlinnPhongTex = 0,
	CubeDepthMap,
	DepthMap,
	Normals,
	Shadow,
	ShadowPoint,
	SimpleColor,
	SimpleExtrude,
	Screen,
	SkyBox,
	SkyBoxPanorama,
	VarianceShadow
};

/**
* Maps shader enumerations to a string representation.
*/
const static platform::util::EnumString<Shaders> shaderEnumConversion[] = {
	{ Shaders::BlinnPhongTex, "BLINN_PHONG_TEX" },
	{ Shaders::CubeDepthMap, "CUBE_DEPTH_MAP" },
	{ Shaders::DepthMap, "DEPTH_MAP" },
	{ Shaders::Normals, "NORMALS" },
	{ Shaders::Shadow, "SHADOW" },
	{ Shaders::ShadowPoint, "SHADOW_POINT" },
	{ Shaders::SimpleColor, "SIMPLE_COLOR" },
	{ Shaders::SimpleExtrude, "SIMPLE_EXTRUDE" },
	{ Shaders::Screen, "SCREEN" },
	{ Shaders::SkyBox, "SKY_BOX" },
	{ Shaders::SkyBoxPanorama, "SKY_BOX_PANORAMA" },
	{ Shaders::VarianceShadow, "VARIANCE_SHADOW" }
};

/**
* Maps a string to a shader enum.
* @param str: The string to be mapped
* @return: The mapped shader enum.
*
* ATTENTION: If the string couldn't be mapped, a EnumFormatException
* will be thrown.
*/
static Shaders stringToShaderEnum(const std::string& str)
{
	return stringToEnum(str, shaderEnumConversion);
}

/**
* Puts a string representation of a shader enum to an output stream.
*/
std::ostream& operator<<(std::ostream& os, Shaders shader);

enum class ShaderAttributeType
{
	FLOAT,
	INT,
	MAT4X4,
	TEXTURE2D,
	VEC2,
	VEC3
};

class ShaderAttribute
{
public:
	ShaderAttribute();
	ShaderAttribute(ShaderAttributeType type, const void* data);
	virtual ~ShaderAttribute();

	const void* getData() const;
	ShaderAttributeType getType() const;

	void setData(ShaderAttributeType type, const void* data);

protected:
	const void* data;
	ShaderAttributeType type;
};

class ShaderConfig
{
public:
	using AttributeList = std::list<ShaderAttribute>;

	ShaderConfig();
	virtual ~ShaderConfig();

	void addAttribute(const ShaderAttribute& attribute);

	const AttributeList* getAttributeList() const;

protected:
	std::list<ShaderAttribute> attributes;
};

class Shader
{
public:

	struct TransformData
	{
		glm::mat4 const* projection; 
		glm::mat4 const* view;
		glm::mat4 const* model;
	};

	Shader(): config(nullptr){};
	
	virtual ~Shader(){}

	virtual void draw(Mesh const& mesh) = 0;
	
	virtual void drawInstanced(Mesh const& mesh, unsigned int amount) = 0;

	virtual void release() = 0;

	void setConfig(const ShaderConfig* config)
	{
		this->config = config;
	}

	void setTransformData(TransformData data)
	{
		this->data = std::move(data);
	}

protected:
	const ShaderConfig* config;
	TransformData data;
};