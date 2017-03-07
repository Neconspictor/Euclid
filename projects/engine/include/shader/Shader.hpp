#pragma once

#include <mesh/Mesh.hpp>
#include <glm/glm.hpp>
#include <platform/util/StringUtils.hpp>

struct TransformData;

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
static Shaders stringToShaderEnum(const std::string& str);

/**
* Puts a string representation of a shader enum to an output stream.
*/
std::ostream& operator<<(std::ostream& os, Shaders shader);

enum class ShaderAttributeType
{
	CubeMap,
	FLOAT,
	INT,
	MAT4X4,
	TEXTURE2D,
	VEC2,
	VEC3,
	VEC4
};

class ShaderAttribute
{
public:
	virtual ~ShaderAttribute();

	void activate(bool active);

	const void* getData() const;
	ShaderAttributeType getType() const;

	bool isActive() const;

protected:
	const void* data;
	bool m_isActive;
	ShaderAttributeType type;

	ShaderAttribute(); // This class is not intended to be instanced, except by derived classes
};


class ShaderConfig
{
public:
	ShaderConfig();
	virtual ~ShaderConfig();

	virtual const ShaderAttribute* getAttributeList() const = 0;
	virtual int getNumberOfAttributes() const = 0;

	virtual void update(const TransformData& data) = 0;
};

class Shader
{
public:

	Shader();
	
	virtual ~Shader();

	virtual void afterDrawing();

	virtual void beforeDrawing();

	virtual void draw(Mesh const& mesh) = 0;
	
	virtual void drawInstanced(Mesh const& mesh, unsigned int amount) = 0;

	virtual const ShaderConfig* getConfig() const = 0;

	virtual void release() = 0;

	void setTransformData(TransformData data);

protected:
	TransformData data;
};

struct TransformData
{
	glm::mat4 const* projection;
	glm::mat4 const* view;
	glm::mat4 const* model;
};