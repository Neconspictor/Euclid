#pragma once
#include <platform/util/StringUtils.hpp>

/**
* Enumerates all shaders that can be used for shading models.
*/
enum class Shaders
{
	BlinnPhongTex = 0,
	CubeDepthMap,
	DepthMap,
	Lamp,
	Normals,
	Phong,
	PhongTex,
	Playground,
	Shadow,
	ShadowPoint,
	SimpleColor,
	SimpleExtrude,
	SimpleLight,
	SimpleReflection,
	Screen,
	SkyBox,
	SkyBoxPanorama,
	VarianceShadow
};

/**
* Maps shader enumerations to a string representation.
*/
const static platform::util::EnumString<Shaders> shaderEnumConversion[] = {
	{Shaders::BlinnPhongTex, "BLINN_PHONG_TEX"},
	{Shaders::CubeDepthMap, "CUBE_DEPTH_MAP"},
	{Shaders::DepthMap, "DEPTH_MAP"},
	{Shaders::Lamp, "LAMP"},
	{Shaders::Normals, "NORMALS"},
	{Shaders::Phong, "PHONG"},
	{Shaders::PhongTex, "PHONG_TEX"},
	{Shaders::Playground, "PLAYGROUND"},
	{Shaders::Shadow, "SHADOW"},
	{Shaders::ShadowPoint, "SHADOW_POINT"},
	{Shaders::SimpleColor, "SIMPLE_COLOR"},
	{Shaders::SimpleExtrude, "SIMPLE_EXTRUDE"},
	{Shaders::SimpleLight, "SIMPLE_LIGHT"},
	{Shaders::SimpleReflection, "SIMPLE_REFLECTION"},
	{Shaders::Screen, "SCREEN"},
	{Shaders::SkyBox, "SKY_BOX"},
	{Shaders::SkyBoxPanorama, "SKY_BOX_PANORAMA"},
	{Shaders::VarianceShadow, "VARIANCE_SHADOW"}
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