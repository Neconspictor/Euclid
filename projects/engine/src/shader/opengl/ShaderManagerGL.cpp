#include <shader/opengl/ShaderManagerGL.hpp>
#include <shader/opengl/PlaygroundShaderGL.hpp>
#include <shader/opengl/LampShaderGL.hpp>
#include <shader/opengl/SimpleLightShaderGL.hpp>
#include <sstream>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <exception/ShaderInitException.hpp>
#include <shader/opengl/PhongShaderGL.hpp>
#include <shader/opengl/PhongTexShaderGL.hpp>
#include <shader/opengl/SimpleColorShaderGL.hpp>
#include <shader/opengl/SimpleExtrudeShaderGL.hpp>
#include <shader/opengl/ScreenShaderGL.hpp>
#include <shader/opengl/SkyBoxShaderGL.hpp>
#include <shader/opengl/SimpleReflectionShaderGL.hpp>
#include <shader/opengl/NormalsShaderGL.hpp>
#include <shader/opengl/ShadowShaderGL.hpp>
#include <shader/opengl/DepthMapShaderGL.hpp>

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
	createShader(s::DepthMap);
	createShader(s::Lamp);
	createShader(s::Normals);
	createShader(s::Phong);
	createShader(s::PhongTex);
	createShader(s::Playground);
	createShader(s::Shadow);
	createShader(s::ShadowPoint);
	createShader(s::SimpleColor);
	createShader(s::SimpleExtrude);
	createShader(s::SimpleLight);
	createShader(s::SimpleReflection);
	createShader(s::Screen);
	createShader(s::SkyBox);
	createShader(s::SkyBoxPanorama);
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
		shaderPtr = make_shared<PhongTexShaderGL>("blinn_phong_tex_mult_lights_vs.glsl", "blinn_phong_tex_mult_lights_fs.glsl",
			"blinn_phong_tex_mult_lights_vs_Instanced.glsl");
		break;
	}
	case s::DepthMap: {
		shaderPtr = make_shared<DepthMapShaderGL>("depth_map_vs.glsl", "depth_map_fs.glsl");
		break;
	}
	case s::Lamp: {
		shaderPtr = make_shared<LampShaderGL>("lamp_vs.glsl", "lamp_fs.glsl");
		break;
	}
	case s::Normals: {
		shaderPtr = make_shared<NormalsShaderGL>("normals_vs.glsl", "normals_fs.glsl", "normals_gs.glsl");
		break;
	}
	case s::Phong: {
		shaderPtr = make_shared<PhongShaderGL>("phong_vs.glsl", "phong_fs.glsl");
		break;
	}
	case s::PhongTex: {
		shaderPtr = make_shared<PhongTexShaderGL>("phong_tex_mult_lights_vs.glsl", "phong_tex_mult_lights_fs.glsl", 
			"phong_tex_mult_lights_vs_Instanced.glsl");
		break;
	}
	case s::Playground: {
		shaderPtr = make_shared<PlaygroundShaderGL>("playground_vs.glsl", "playground_fs.glsl", "playground_gs.glsl");
		break;
	}
	case s::Shadow: {
		shaderPtr = make_shared<ShadowShaderGL>
			("shadow_vs.glsl", "shadow_fs.glsl");
		break;
	}
	case s::ShadowPoint: {
		shaderPtr = make_shared<PointShadowShaderGL>
			("shadow_point_vs.glsl", "shadow_point_fs.glsl", "shadow_point_gs.glsl");
		break;
	}
	case s::SimpleColor: {
		shaderPtr = make_shared<SimpleColorShaderGL>
			("simpleColor_vs.glsl", "simpleColor_fs.glsl");
		break;
	}
	case s::SimpleExtrude: {
		shaderPtr = make_shared<SimpleExtrudeShaderGL>
			("simpleExtrude_vs.glsl", "simpleExtrude_fs.glsl");
		break;
	}
	case s::SimpleLight: {
		shaderPtr = make_shared<SimpleLightShaderGL>
			("simpleLight_vs.glsl", "simpleLight_fs.glsl");
		break;
	}
	case s::SimpleReflection: {
		shaderPtr = make_shared<SimpleReflectionShaderGL>
			("simpleReflection_vs.glsl", "simpleReflection_fs.glsl");
		break;
	}
	case s::Screen: {
		shaderPtr = make_shared<ScreenShaderGL>
			("screen_vs.glsl", "screen_fs.glsl");
		break;
	}
	case s::SkyBox: {
		shaderPtr = make_shared<SkyBoxShaderGL>
			("skybox_vs.glsl", "skybox_fs.glsl");
		break;
	}
	case s::SkyBoxPanorama: {
		shaderPtr = make_shared<PanoramaSkyBoxShaderGL>
			("panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");
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