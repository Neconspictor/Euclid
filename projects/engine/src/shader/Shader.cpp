#include <shader/Shader.hpp>
#include <platform/util/StringUtils.hpp>

using namespace std;

ShaderAttribute::~ShaderAttribute() {}

void ShaderAttribute::activate(bool active)
{
	m_isActive = active;
}

const void* ShaderAttribute::getData() const
{
	return data;
}

ShaderAttributeType ShaderAttribute::getType() const
{
	return type;
}

bool ShaderAttribute::isActive() const
{
	return m_isActive;
}

ShaderAttribute::ShaderAttribute() : data(nullptr), m_isActive(true), type(ShaderAttributeType::MAT4X4)
{}

ShaderConfig::ShaderConfig(){}

ShaderConfig::~ShaderConfig(){}

Shader::Shader() {}

Shader::~Shader() {}

void Shader::afterDrawing() {}

void Shader::beforeDrawing() {}

void Shader::setTransformData(TransformData data)
{
	this->data = move(data);
}

Shaders stringToShaderEnum(const string& str)
{
	return stringToEnum(str, shaderEnumConversion);
}

ostream& operator<<(ostream& os, Shaders shader)
{
	os << enumToString(shader, shaderEnumConversion);
	return os;
}