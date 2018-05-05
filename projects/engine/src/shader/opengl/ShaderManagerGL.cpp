#include <shader/opengl/ShaderManagerGL.hpp>
#include <sstream>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <exception/ShaderInitException.hpp>
#include <shader/opengl/PBRShaderGL.hpp>
#include <shader/opengl/PhongTexShaderGL.hpp>
#include <shader/opengl/SimpleColorShaderGL.hpp>
#include <shader/opengl/SimpleExtrudeShaderGL.hpp>
#include <shader/opengl/ScreenShaderGL.hpp>
#include <shader/opengl/SkyBoxShaderGL.hpp>
#include <shader/opengl/NormalsShaderGL.hpp>
#include <shader/opengl/ShadowShaderGL.hpp>
#include <shader/opengl/DepthMapShaderGL.hpp>
#include <shader/opengl/post_processing/blur/GaussianBlurShaderGL.hpp>

using namespace std;
using namespace platform;

unique_ptr<ShaderManagerGL> ShaderManagerGL::instance = make_unique<ShaderManagerGL>(ShaderManagerGL());

ShaderManagerGL::ShaderManagerGL() : 
	logClient(getLogServer())
{
}

ShaderManagerGL::~ShaderManagerGL()
{
}

ShaderConfig* ShaderManagerGL::getConfig(Shaders shader)
{
	return getShader(shader)->getConfig();
}

Shader* ShaderManagerGL::getShader(Shaders shaderEnum)
{
	auto it = shaderMap.find(shaderEnum);
	if (it == shaderMap.end())
	{
		LOG(logClient, Debug) << "Create singleton for shader: " << shaderEnum;
		return createShader(shaderEnum);
	}

	return it->second.get();
}

void ShaderManagerGL::loadShaders()
{
	using s = Shaders;
	createShader(s::BlinnPhongTex);
	createShader(s::Pbr);
	createShader(s::Pbr_Convolution);
	createShader(s::Pbr_Prefilter);
	createShader(s::CubeDepthMap);
	createShader(s::DepthMap);
	createShader(s::GaussianBlurHorizontal);
	createShader(s::GaussianBlurVertical);
	createShader(s::Normals);
	createShader(s::Shadow);
	createShader(s::ShadowPoint);
	createShader(s::SimpleColor);
	createShader(s::SimpleExtrude);
	createShader(s::Screen);
	createShader(s::SkyBox);
	createShader(s::SkyBoxEquirectangular);
	createShader(s::SkyBoxPanorama);
	createShader(s::VarianceDepthMap);
	createShader(s::VarianceShadow);
}

void ShaderManagerGL::validateShader(Shader* shader)
{
	if (!dynamic_cast<ShaderGL*>(shader))
	{
		throw runtime_error("ShaderManagerGL::validateShader(Shader*): Shader isn't a valid OpenGL shader!");
	}
}

ShaderManagerGL* ShaderManagerGL::get()
{
	return instance.get();
}

Shader* ShaderManagerGL::createShader(Shaders shaderEnum)
{
	using s = Shaders;
	shared_ptr<Shader> shaderPtr;
	switch(shaderEnum)
	{
	case s::BlinnPhongTex: {
		configs.push_back(make_shared<PhongTexShaderGL>());
		shaderPtr = make_shared<ShaderGL>(configs.back().get(), "blinn_phong_tex_mult_lights_vs.glsl", "blinn_phong_tex_mult_lights_fs.glsl");
		break;
	}
	case s::Pbr: {
		configs.push_back(make_shared<PBRShaderGL>());
		shaderPtr = make_shared<ShaderGL>(configs.back().get(), "pbr_vs.glsl", "pbr_fs.glsl");
		break;
	}
	case s::Pbr_Convolution: {
		configs.push_back(make_shared<PBR_ConvolutionShaderGL>());
		shaderPtr = make_shared<ShaderGL>(configs.back().get(), "pbr/pbr_convolution_vs.glsl", "pbr/pbr_convolution_fs.glsl");
		break;
	}
	case s::Pbr_Prefilter: {
		configs.push_back(make_shared<PBR_PrefilterShaderGL>());
		shaderPtr = make_shared<ShaderGL>(configs.back().get(), "pbr/pbr_prefilter_cubemap_vs.glsl", "pbr/pbr_prefilter_cubemap_fs.glsl");
		break;
	}
	case s::CubeDepthMap: {
		configs.push_back(make_shared<CubeDepthMapShaderGL>());
		shaderPtr = make_shared<ShaderGL>(configs.back().get(),
			"depth_map_cube_vs.glsl", "depth_map_cube_fs.glsl");
		break;
	}
	case s::DepthMap: {
		configs.push_back(make_shared<DepthMapShaderGL>());
		shaderPtr = make_shared<ShaderGL>(configs.back().get(), "depth_map_vs.glsl", 
			"depth_map_fs.glsl");
		break;
	}
	case s::GaussianBlurHorizontal: {
		configs.push_back(make_shared<GaussianBlurHorizontalShaderGL>());
		shaderPtr = make_shared<ShaderGL>(configs.back().get(), "post_processing/blur/gaussian_blur_vs.glsl",
			"post_processing/blur/gaussian_blur_horizontal_fs.glsl");
		break;
	}
	case s::GaussianBlurVertical: {
		configs.push_back(make_shared<GaussianBlurVerticalShaderGL>());
		shaderPtr = make_shared<ShaderGL>(configs.back().get(), "post_processing/blur/gaussian_blur_vs.glsl",
			"post_processing/blur/gaussian_blur_vertical_fs.glsl");
		break;
	}
	case s::Normals: {
		configs.push_back(make_shared<NormalsShaderGL>());
		shaderPtr = make_shared<ShaderGL>(configs.back().get(), "normals_vs.glsl", 
			"normals_fs.glsl", "normals_gs.glsl");
		break;
	}
	case s::Shadow: {
		configs.push_back(make_shared<ShadowShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "shadow_vs.glsl", "shadow_fs.glsl");
		break;
	}
	case s::ShadowPoint: {
		configs.push_back(make_shared<PointShadowShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "shadow_point_vs.glsl", "shadow_point_fs.glsl", 
				"shadow_point_gs.glsl");
		break;
	}
	case s::SimpleColor: {
		configs.push_back(make_shared<SimpleColorShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "simpleColor_vs.glsl", "simpleColor_fs.glsl");
		break;
	}
	case s::SimpleExtrude: {
		configs.push_back(make_shared<SimpleExtrudeShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "simpleExtrude_vs.glsl", "simpleExtrude_fs.glsl");
		break;
	}
	case s::Screen: {
		configs.push_back(make_shared<ScreenShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "screen_vs.glsl", "screen_fs.glsl");
		break;
	}
	case s::SkyBox: {
		configs.push_back(make_shared<SkyBoxShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "skybox_vs.glsl", "skybox_fs.glsl");
		break;
	}
	case s::SkyBoxEquirectangular: {
		configs.push_back(make_shared<EquirectangularSkyBoxShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "skybox_equirectangular_vs.glsl", "skybox_equirectangular_fs.glsl");
		break;
	}
	case s::SkyBoxPanorama: {
		configs.push_back(make_shared<PanoramaSkyBoxShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");
		break;
	}
	case s::VarianceDepthMap: {
		configs.push_back(make_shared<VarianceDepthMapShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "variance_depth_map_vs.glsl", "variance_depth_map_fs.glsl");
		break;
	}
	case s::VarianceShadow: {
		configs.push_back(make_shared<VarianceShadowShaderGL>());
		shaderPtr = make_shared<ShaderGL>
			(configs.back().get(), "variance_shadow_vs.glsl", "variance_shadow_fs.glsl");
		break;
	}
	default: {
		stringstream ss;
		ss << BOOST_CURRENT_FUNCTION << " : couldn't create shader for: " << shaderEnum;
		throw ShaderInitException(ss.str());
	}
	}
	
	Shader* result = shaderPtr.get();
	assert(result != nullptr);
	shaderMap[shaderEnum] = shaderPtr;
	return result;
}