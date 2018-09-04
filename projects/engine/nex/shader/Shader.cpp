#include <nex/shader/Shader.hpp>
#include <nex/util/StringUtils.hpp>

using namespace std;


/**
* Maps shader enumerations to a string representation.
*/
const static nex::util::EnumString<Shaders> shaderEnumConversion[] = {
	{ Shaders::BlinnPhongTex, "BLINN_PHONG_TEX" },
{ Shaders::Pbr, "PBR" },
{ Shaders::Pbr_Deferred_Geometry, "PBR_DEFERRED_GEOMETRY" },
{ Shaders::Pbr_Deferred_Lighting, "PBR_DEFERRED_LIGHTING" },
{ Shaders::Pbr_Convolution, "PBR_CONVOLUTION" },
{ Shaders::Pbr_Prefilter, "PBR_PREFILTER" },
{ Shaders::Pbr_BrdfPrecompute, "PBR_BRDF_PRECOMPUTE" },
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

ShaderAttribute::ShaderAttribute() : data(nullptr), m_isActive(true), type(ShaderAttributeType::MAT4)
{}


ShaderConfig::~ShaderConfig(){}

Shader::~Shader() {}

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