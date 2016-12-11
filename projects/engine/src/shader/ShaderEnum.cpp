#include <shader/ShaderEnum.hpp>
#include <platform/util/StringUtils.hpp>

std::ostream& operator<<(std::ostream& os, ShaderEnum shader)
{
	os << enumToString(shader, shaderEnumConversion);
	return os;
}