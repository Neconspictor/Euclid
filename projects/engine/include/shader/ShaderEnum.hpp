#ifndef ENGINE_SHADER_SHADER_ENUM_HPP
#define ENGINE_SHADER_SHADER_ENUM_HPP
#include <platform/util/StringUtils.hpp>

/**
* Enumerates all shaders that can be used for shading models.
*/
enum ShaderEnum
{
	Lamp = 0,
	Phong,
	PhongTex,
	Playground,
	SimpleLight
};

/**
* Maps shader enumerations to a string representation.
*/
const static platform::util::EnumString<ShaderEnum> shaderEnumConversion[] = {
	{Lamp, "LAMP" },
	{Phong, "PHONG"},
	{ PhongTex, "PHONG_TEX" },
	{Playground, "PLAYGROUND" },
	{SimpleLight, "SIMPLE_LIGHT" },
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
	std::ostream& operator<<(std::ostream& os, const ShaderEnum& shader);
#endif