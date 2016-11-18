#include <shader/ShaderEnum.hpp>
#include <platform/exception/EnumFormatException.hpp>

std::ostream& operator<<(std::ostream& os, const ShaderEnum& shader)
{
	switch (shader)
	{
	case Lamp: os << "LAMP"; break;
	case Playground: os << "PLAYGROUND"; break;
	case SimpleLight: os << "SIMPLE_LIGHT"; break;
	default:
		throw EnumFormatException(BOOST_CURRENT_FUNCTION + std::string(": Unknown shader enum: " + shader));
	}
	return os;
}