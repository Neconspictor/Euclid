#include <shader/ShaderEnum.hpp>
#include <platform/util/StringUtils.hpp>

std::ostream& operator<<(std::ostream& os, Shaders shader)
{
	os << enumToString(shader, shaderEnumConversion);
	return os;
}