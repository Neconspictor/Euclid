#pragma once
#include <platform/util/StringUtils.hpp>

/**
* Enumerates all shaders that can be used for shading models.
*/
enum ShaderEnum
{
	BlinnPhongTex = 0,
	Lamp,
	Normals,
	Phong,
	PhongTex,
	Playground,
	Shadow,
	SimpleColor,
	SimpleExtrude,
	SimpleLight,
	SimpleReflection,
	Screen,
	SkyBox,
	SkyBoxPanorama
};

/**
* Maps shader enumerations to a string representation.
*/
const static platform::util::EnumString<ShaderEnum> shaderEnumConversion[] = {
	{BlinnPhongTex, "BLINN_PHONG_TEX" },
	{Lamp, "LAMP"},
	{Normals, "NORMALS" },
	{Phong, "PHONG"},
	{PhongTex, "PHONG_TEX"},
	{Playground, "PLAYGROUND"},
	{Shadow, "SHADOW" },
	{SimpleColor, "SIMPLE_COLOR"},
	{SimpleExtrude, "SIMPLE_EXTRUDE"},
	{SimpleLight, "SIMPLE_LIGHT"},
	{SimpleReflection, "SIMPLE_REFLECTION"},
	{Screen, "SCREEN"},
	{SkyBox, "SKY_BOX"},
	{SkyBoxPanorama, "SKY_BOX_PANORAMA" }
};

	/**
	* Maps a string to a shader enum.
	* @param str: The string to be mapped
	* @return: The mapped shader enum.
	*
	* ATTENTION: If the string couldn't be mapped, a EnumFormatException
	* will be thrown.
	*/
	static ShaderEnum stringToShaderEnum(const std::string& str)
	{
		return stringToEnum(str, shaderEnumConversion);
	}

	/**
	* Puts a string representation of a shader enum to an output stream.
	*/
	std::ostream& operator<<(std::ostream& os, ShaderEnum shader);