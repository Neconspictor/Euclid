#pragma once

#include <nex/mesh/Mesh.hpp>
#include <glm/glm.hpp>

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
	Pbr_Deferred_Geometry,
	Pbr_Deferred_Lighting,
	Pbr_Convolution,
	Pbr_Prefilter,
	Pbr_BrdfPrecompute,
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
	TEXTURE2D_ARRAY,
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

	virtual void use() = 0;

	void setTransformData(TransformData data);

protected:
	TransformData data;
};