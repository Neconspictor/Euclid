#pragma once

#include <mesh/Mesh.hpp>
#include <glm/glm.hpp>
#include <platform/util/StringUtils.hpp>

struct TransformData
{
	glm::mat4 const* projection;
	glm::mat4 const* view;
	glm::mat4 const* model;
};

/**
* Enumerates all shaders that can be used for shading models.
*/
enum class Shaders
{
	Unknown = 0,
	BlinnPhongTex,
	Pbr,
	Pbr_Convolution,
	CubeDepthMap,
	DepthMap,
	GaussianBlurHorizontal,
	GaussianBlurVertical,
	Normals,
	Shadow,
	ShadowPoint,
	SimpleColor,
	SimpleExtrude,
	Screen,
	SkyBox,
	SkyBoxEquirectangular,
	SkyBoxPanorama,
	VarianceDepthMap,
	VarianceShadow
};

/**
* Maps shader enumerations to a string representation.
*/
const static platform::util::EnumString<Shaders> shaderEnumConversion[] = {
	{ Shaders::BlinnPhongTex, "BLINN_PHONG_TEX" },
	{ Shaders::Pbr, "PBR" },
	{ Shaders::Pbr_Convolution, "PBR_CONVOLUTION" },
	{ Shaders::CubeDepthMap, "CUBE_DEPTH_MAP" },
	{ Shaders::DepthMap, "DEPTH_MAP" },
	{ Shaders::GaussianBlurHorizontal, "GAUSSIAN_BLUR_HORIZONTAL" },
	{ Shaders::GaussianBlurVertical, "GAUSSIAN_BLUR_VERTICAL" },
	{ Shaders::Normals, "NORMALS" },
	{ Shaders::Shadow, "SHADOW" },
	{ Shaders::ShadowPoint, "SHADOW_POINT" },
	{ Shaders::SimpleColor, "SIMPLE_COLOR" },
	{ Shaders::SimpleExtrude, "SIMPLE_EXTRUDE" },
	{ Shaders::Screen, "SCREEN" },
	{ Shaders::SkyBox, "SKY_BOX" },
	{ Shaders::SkyBoxEquirectangular, "SKY_BOX_EQUIRECTANGULAR" },
	{ Shaders::SkyBoxPanorama, "SKY_BOX_PANORAMA" },
	{ Shaders::VarianceShadow, "VARIANCE_DEPTH_MAP" },
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
	CUBE_MAP,
	FLOAT,
	INT,
	MAT3,
	MAT4,
	TEXTURE2D,
	VEC2,
	VEC3,
	VEC4
};

class ShaderAttribute
{
public:
	ShaderAttribute();

	virtual ~ShaderAttribute();

	void activate(bool active);

	const void* getData() const;
	ShaderAttributeType getType() const;

	bool isActive() const;

protected:
	const void* data;
	bool m_isActive;
	ShaderAttributeType type;
};


class ShaderConfig
{
public:	
	virtual ~ShaderConfig();
};

class Shader
{
public:	
	virtual ~Shader();

	virtual void draw(Mesh const& mesh) = 0;
	
	virtual void drawInstanced(Mesh const& mesh, unsigned int amount) = 0;

	virtual ShaderConfig* getConfig() const = 0;

	virtual void release() = 0;

	void setTransformData(TransformData data);

protected:
	TransformData data;
};