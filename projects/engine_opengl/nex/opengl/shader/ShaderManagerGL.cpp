#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <sstream>
#include <nex/logging/GlobalLoggingServer.hpp>
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
	logClient(getLogServer())
{
}

ShaderManagerGL::~ShaderManagerGL()
{
}

ShaderConfigGL* ShaderManagerGL::getConfig(Shaders shader)
{
	return getShader(shader)->getConfig();
}

ShaderGL* ShaderManagerGL::getShader(Shaders shaderEnum)
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
	createShader(s::Pbr_Deferred_Geometry);
	createShader(s::Pbr_Deferred_Lighting);
	createShader(s::Pbr_Convolution);
	createShader(s::Pbr_Prefilter);
	createShader(s::Pbr_BrdfPrecompute);
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

void ShaderManagerGL::validateShader(ShaderGL* shader)
{
	if (!dynamic_cast<ShaderGL*>(shader))
	{
		throw_with_trace(runtime_error("ShaderManagerGL::validateShader(Shader*): Shader isn't a valid OpenGL shader!"));
	}
}

ShaderManagerGL* ShaderManagerGL::get()
{
	return instance.get();
}

ShaderGL* ShaderManagerGL::createShader(Shaders shaderEnum)
{
	using s = Shaders;
	shared_ptr<ShaderGL> shaderPtr;
	switch(shaderEnum)
	{
	case s::BlinnPhongTex: {
		std::unique_ptr<PhongTexShaderGL> config = make_unique<PhongTexShaderGL>();
		shaderPtr = make_shared<ShaderGL>(move(config), "blinn_phong_tex_mult_lights_vs.glsl", "blinn_phong_tex_mult_lights_fs.glsl");
		break;
	}
	case s::Pbr: {
		shaderPtr = make_shared<ShaderGL>(make_unique<PBRShaderGL>(), "pbr/pbr_forward_vs.glsl", "pbr/pbr_forward_fs.glsl");
		break;
	}
	case s::Pbr_Deferred_Geometry: {
		shaderPtr = make_shared<ShaderGL>(make_unique<PBRShader_Deferred_GeometryGL>(), 
			"pbr/pbr_deferred_geometry_pass_vs.glsl", "pbr/pbr_deferred_geometry_pass_fs.glsl");
		break;
	}
	case s::Pbr_Deferred_Lighting: {
		shaderPtr = make_shared<ShaderGL>(make_unique<PBRShader_Deferred_LightingGL>(), "pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs.glsl");
		break;
	}
	case s::Pbr_Convolution: {
		shaderPtr = make_shared<ShaderGL>(make_unique<PBR_ConvolutionShaderGL>(), "pbr/pbr_convolution_vs.glsl", "pbr/pbr_convolution_fs.glsl");
		break;
	}
	case s::Pbr_Prefilter: {
		shaderPtr = make_shared<ShaderGL>(make_unique<PBR_PrefilterShaderGL>(), "pbr/pbr_prefilter_cubemap_vs.glsl", "pbr/pbr_prefilter_cubemap_fs.glsl");
		break;
	}
	case s::Pbr_BrdfPrecompute: {
		shaderPtr = make_shared<ShaderGL>(make_unique<PBR_BrdfPrecomputeShaderGL>(), "pbr/pbr_brdf_precompute_vs.glsl", "pbr/pbr_brdf_precompute_fs.glsl");
		break;
	}
	case s::CubeDepthMap: {
		shaderPtr = make_shared<ShaderGL>(make_unique<CubeDepthMapShaderGL>(),
			"depth_map_cube_vs.glsl", "depth_map_cube_fs.glsl");
		break;
	}
	case s::DepthMap: {
		shaderPtr = make_shared<ShaderGL>(make_unique<DepthMapShaderGL>(), "depth_map_vs.glsl",
			"depth_map_fs.glsl");
		break;
	}
	case s::GaussianBlurHorizontal: {
		shaderPtr = make_shared<ShaderGL>(make_unique<GaussianBlurHorizontalShaderGL>(), "post_processing/blur/gaussian_blur_vs.glsl",
			"post_processing/blur/gaussian_blur_horizontal_fs.glsl");
		break;
	}
	case s::GaussianBlurVertical: {
		shaderPtr = make_shared<ShaderGL>(make_unique<GaussianBlurVerticalShaderGL>(), "post_processing/blur/gaussian_blur_vs.glsl",
			"post_processing/blur/gaussian_blur_vertical_fs.glsl");
		break;
	}
	case s::Normals: {
		shaderPtr = make_shared<ShaderGL>(make_unique<NormalsShaderGL>(), "normals_vs.glsl",
			"normals_fs.glsl", "normals_gs.glsl");
		break;
	}
	case s::Shadow: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<ShadowShaderGL>(), "shadow_vs.glsl", "shadow_fs.glsl");
		break;
	}
	case s::ShadowPoint: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<PointShadowShaderGL>(), "shadow_point_vs.glsl", "shadow_point_fs.glsl",
				"shadow_point_gs.glsl");
		break;
	}
	case s::SimpleColor: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<SimpleColorShaderGL>(), "simpleColor_vs.glsl", "simpleColor_fs.glsl");
		break;
	}
	case s::SimpleExtrude: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<SimpleExtrudeShaderGL>(), "simpleExtrude_vs.glsl", "simpleExtrude_fs.glsl");
		break;
	}
	case s::Screen: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<ScreenShaderGL>(), "screen_vs.glsl", "screen_fs.glsl");
		break;
	}
	case s::SkyBox: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<SkyBoxShaderGL>(), "skybox_vs.glsl", "skybox_fs.glsl");
		break;
	}
	case s::SkyBoxEquirectangular: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<EquirectangularSkyBoxShaderGL>(), "skybox_equirectangular_vs.glsl", "skybox_equirectangular_fs.glsl");
		break;
	}
	case s::SkyBoxPanorama: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<PanoramaSkyBoxShaderGL>(), "panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");
		break;
	}
	case s::VarianceDepthMap: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<VarianceDepthMapShaderGL>(), "variance_depth_map_vs.glsl", "variance_depth_map_fs.glsl");
		break;
	}
	case s::VarianceShadow: {
		shaderPtr = make_shared<ShaderGL>
			(make_unique<VarianceShadowShaderGL>(), "variance_shadow_vs.glsl", "variance_shadow_fs.glsl");
		break;
	}
	default: {
		stringstream ss;
		ss << BOOST_CURRENT_FUNCTION << " : couldn't create shader for: " << shaderEnum;
		throw_with_trace(ShaderInitException(ss.str()));
	}
	}
	
	ShaderGL* result = shaderPtr.get();
	assert(result != nullptr);
	shaderMap[shaderEnum] = shaderPtr;
	return result;
}