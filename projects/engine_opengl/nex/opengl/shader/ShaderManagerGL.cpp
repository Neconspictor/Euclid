#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <sstream>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/opengl/shader/PBRShaderGL.hpp>
#include <nex/opengl/shader/ScreenShaderGL.hpp>
#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <nex/opengl/shader/ShadowShaderGL.hpp>
#include <nex/opengl/shader/DepthMapShaderGL.hpp>
#include <nex/opengl/shader/post_processing/blur/GaussianBlurShaderGL.hpp>
#include <nex/util/ExceptionHandling.hpp>

using namespace std;
using namespace nex;

unique_ptr<ShaderManagerGL> ShaderManagerGL::instance = make_unique<ShaderManagerGL>(ShaderManagerGL());

ShaderManagerGL::ShaderManagerGL() : 
	m_logger("ShaderManagerGL")
{
}

ShaderManagerGL::~ShaderManagerGL()
{
	for (auto& it : shaderMap)
		delete it.second;
	shaderMap.clear();
}

Shader* ShaderManagerGL::getShader(ShaderType shaderEnum)
{
	auto it = shaderMap.find(shaderEnum);
	if (it == shaderMap.end())
	{
		LOG(m_logger, Debug) << "Create singleton for shader: " << shaderEnum;
		return createShader(shaderEnum);
	}

	return it->second;
}

void ShaderManagerGL::loadShaders()
{
	using s = ShaderType;
	createShader(s::Pbr);
	createShader(s::Pbr_Deferred_Geometry);
	createShader(s::Pbr_Deferred_Lighting);
	createShader(s::Pbr_Convolution);
	createShader(s::Pbr_Prefilter);
	createShader(s::Pbr_BrdfPrecompute);
	createShader(s::CubeDepthMap);
	createShader(s::DepthMap);
	createShader(s::GaussianBlurHorizontal);
	createShader(s::GaussianBlurVertical);
	createShader(s::Shadow);
	createShader(s::Screen);
	createShader(s::SkyBox);
	createShader(s::SkyBoxEquirectangular);
	createShader(s::SkyBoxPanorama);
}

ShaderManagerGL* ShaderManagerGL::get()
{
	return instance.get();
}

Shader* ShaderManagerGL::createShader(ShaderType shaderEnum)
{
	using s = ShaderType;
	Shader* shaderPtr = nullptr;
	switch(shaderEnum)
	{
	case s::Pbr: {
		shaderPtr = new PBRShader();
		break;
	}
	case s::Pbr_Deferred_Geometry: {
		shaderPtr = new PBRShader_Deferred_Geometry();
		break;
	}
	case s::Pbr_Deferred_Lighting: {
		shaderPtr = new PBRShader_Deferred_Lighting();
		break;
	}
	case s::Pbr_Convolution: {
		shaderPtr = new PBR_ConvolutionShader();
		break;
	}
	case s::Pbr_Prefilter: {
		shaderPtr = new PBR_PrefilterShader();
		break;
	}
	case s::Pbr_BrdfPrecompute: {
		shaderPtr = new PBR_BrdfPrecomputeShader();
		break;
	}
	case s::CubeDepthMap: {
		shaderPtr = new CubeDepthMapShader();
		break;
	}
	case s::DepthMap: {
		shaderPtr = new DepthMapShader();
		break;
	}
	case s::GaussianBlurHorizontal: {
		shaderPtr = new GaussianBlurHorizontalShader();
		break;
	}
	case s::GaussianBlurVertical: {
		shaderPtr = new GaussianBlurVerticalShader();
		break;
	}
	case s::Shadow: {
		shaderPtr = new ShadowShader();
		break;
	}
	case s::Screen: {
		shaderPtr = new ScreenShaderGL();
		break;
	}
	case s::SkyBox: {
		shaderPtr = new SkyBoxShader();
		break;
	}
	case s::SkyBoxEquirectangular: {
		shaderPtr = new EquirectangularSkyBoxShader();
		break;
	}
	case s::SkyBoxPanorama: {
		shaderPtr = new PanoramaSkyBoxShader();
		break;
	}
	default: {
		stringstream ss;
		ss << BOOST_CURRENT_FUNCTION << " : couldn't create shader for: " << shaderEnum;
		throw_with_trace(ShaderInitException(ss.str()));
	}
	}
	
	assert(shaderPtr != nullptr);
	shaderMap[shaderEnum] = shaderPtr;
	return shaderPtr;
}