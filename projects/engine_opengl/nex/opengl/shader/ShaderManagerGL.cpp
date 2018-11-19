#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <sstream>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/opengl/shader/PBRShaderGL.hpp>
#include <nex/opengl/shader/PhongTexShaderGL.hpp>
#include <nex/opengl/shader/SimpleColorShaderGL.hpp>
#include <nex/opengl/shader/SimpleExtrudeShaderGL.hpp>
#include <nex/opengl/shader/ScreenShaderGL.hpp>
#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <nex/opengl/shader/NormalsShaderGL.hpp>
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
}

ShaderGL* ShaderManagerGL::getShader(ShaderType shaderEnum)
{
	auto it = shaderMap.find(shaderEnum);
	if (it == shaderMap.end())
	{
		LOG(m_logger, Debug) << "Create singleton for shader: " << shaderEnum;
		return createShader(shaderEnum);
	}

	return it->second.get();
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

void ShaderManagerGL::validateShader(ShaderProgramGL* shader)
{
	if (!dynamic_cast<ShaderProgramGL*>(shader))
	{
		throw_with_trace(runtime_error("ShaderManagerGL::validateShader(Shader*): Shader isn't a valid OpenGL shader!"));
	}
}

ShaderManagerGL* ShaderManagerGL::get()
{
	return instance.get();
}

ShaderGL* ShaderManagerGL::createShader(ShaderType shaderEnum)
{
	using s = ShaderType;
	ShaderGL* shaderPtr = nullptr;
	switch(shaderEnum)
	{
	case s::Pbr: {
		shaderPtr = make_shared<ShaderProgramGL>(make_unique<PBRShaderGL>(), "pbr/pbr_forward_vs.glsl", "pbr/pbr_forward_fs.glsl");
		break;
	}
	case s::Pbr_Deferred_Geometry: {
		shaderPtr = make_shared<ShaderProgramGL>(make_unique<PBRShader_Deferred_GeometryGL>(), 
			"pbr/pbr_deferred_geometry_pass_vs.glsl", "pbr/pbr_deferred_geometry_pass_fs.glsl");
		break;
	}
	case s::Pbr_Deferred_Lighting: {
		shaderPtr = make_shared<ShaderProgramGL>(make_unique<PBRShader_Deferred_LightingGL>(), "pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs.glsl");
		break;
	}
	case s::Pbr_Convolution: {
		shaderPtr = make_shared<ShaderProgramGL>(make_unique<PBR_ConvolutionShaderGL>(), "pbr/pbr_convolution_vs.glsl", "pbr/pbr_convolution_fs.glsl");
		break;
	}
	case s::Pbr_Prefilter: {
		shaderPtr = make_shared<ShaderProgramGL>(make_unique<PBR_PrefilterShaderGL>(), "pbr/pbr_prefilter_cubemap_vs.glsl", "pbr/pbr_prefilter_cubemap_fs.glsl");
		break;
	}
	case s::Pbr_BrdfPrecompute: {
		shaderPtr = make_shared<ShaderProgramGL>(make_unique<PBR_BrdfPrecomputeShaderGL>(), "pbr/pbr_brdf_precompute_vs.glsl", "pbr/pbr_brdf_precompute_fs.glsl");
		break;
	}
	case s::CubeDepthMap: {
		shaderPtr = make_shared<ShaderProgramGL>(make_unique<CubeDepthMapShaderGL>(),
			"depth_map_cube_vs.glsl", "depth_map_cube_fs.glsl");
		break;
	}
	case s::DepthMap: {
		shaderPtr = make_shared<ShaderProgramGL>(make_unique<DepthMapShaderGL>(), "depth_map_vs.glsl",
			"depth_map_fs.glsl");
		break;
	}
	case s::GaussianBlurHorizontal: {
		shaderPtr = new GaussianBlurHorizontalShaderGL();
		break;
	}
	case s::GaussianBlurVertical: {
		shaderPtr = new GaussianBlurVerticalShaderGL();
		break;
	}
	case s::Shadow: {
		shaderPtr = make_shared<ShaderProgramGL>
			(make_unique<ShadowShaderGL>(), "shadow_vs.glsl", "shadow_fs.glsl");
		break;
	}
	case s::Screen: {
		shaderPtr = make_shared<ShaderProgramGL>
			(make_unique<ScreenShaderGL>(), "screen_vs.glsl", "screen_fs.glsl");
		break;
	}
	case s::SkyBox: {
		shaderPtr = make_shared<ShaderProgramGL>
			(make_unique<SkyBoxShaderGL>(), "skybox_vs.glsl", "skybox_fs.glsl");
		break;
	}
	case s::SkyBoxEquirectangular: {
		shaderPtr = make_shared<ShaderProgramGL>
			(make_unique<EquirectangularSkyBoxShaderGL>(), "skybox_equirectangular_vs.glsl", "skybox_equirectangular_fs.glsl");
		break;
	}
	case s::SkyBoxPanorama: {
		shaderPtr = make_shared<ShaderProgramGL>
			(make_unique<PanoramaSkyBoxShaderGL>(), "panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");
		break;
	}
	default: {
		stringstream ss;
		ss << BOOST_CURRENT_FUNCTION << " : couldn't create shader for: " << shaderEnum;
		throw_with_trace(ShaderInitException(ss.str()));
	}
	}
	
	assert(shaderPtr != nullptr);
	shaderMap[shaderEnum].reset(shaderPtr);
	return shaderPtr;
}