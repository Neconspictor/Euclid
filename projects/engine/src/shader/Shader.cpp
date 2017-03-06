#include <shader/Shader.hpp>
#include <platform/util/StringUtils.hpp>

ShaderAttribute::ShaderAttribute() : data(nullptr), type(ShaderAttributeType::MAT4X4)
{}

ShaderAttribute::ShaderAttribute(ShaderAttributeType type, const void* data) : 
	data(data), type(type)
{}

ShaderAttribute::~ShaderAttribute()
{
}

const void* ShaderAttribute::getData() const
{
	return data;
}

ShaderAttributeType ShaderAttribute::getType() const
{
	return type;
}

void ShaderAttribute::setData(ShaderAttributeType type, const void* data)
{
	this->data = data;
	this->type = type;
}

ShaderConfig::ShaderConfig()
{
}

ShaderConfig::~ShaderConfig()
{
}

void ShaderConfig::addAttribute(const ShaderAttribute& attribute)
{
	attributes.push_back(attribute);
}

const ShaderConfig::AttributeList* ShaderConfig::getAttributeList() const
{
	return &attributes;
}

std::ostream& operator<<(std::ostream& os, Shaders shader)
{
	os << enumToString(shader, shaderEnumConversion);
	return os;
}
