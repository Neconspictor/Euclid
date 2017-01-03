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

Shader* ShaderManagerGL::getShader(ShaderEnum shaderEnum)
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
	createShader(Lamp);
	createShader(Normals);
	createShader(Phong);
	createShader(PhongTex);
	createShader(Playground);
	createShader(SimpleColor);
	createShader(SimpleExtrude);
	createShader(SimpleLight);
	createShader(SimpleReflection);
	createShader(Screen);
	createShader(SkyBox);
}

ShaderManagerGL* ShaderManagerGL::get()
{
	return instance.get();
}

Shader* ShaderManagerGL::createShader(ShaderEnum shaderEnum)
{
	shared_ptr<Shader> shaderPtr;
	switch(shaderEnum)
	{
	case Lamp: {
		shaderPtr = make_shared<LampShaderGL>("lamp_vs.glsl", "lamp_fs.glsl");
		break;
	}
	case Normals: {
		shaderPtr = make_shared<NormalsShaderGL>("normals_vs.glsl", "normals_fs.glsl", "normals_gs.glsl");
		break;
	}
	case Phong: {
		shaderPtr = make_shared<PhongShaderGL>("phong_vs.glsl", "phong_fs.glsl");
		break;
	}
	case PhongTex: {
		shaderPtr = make_shared<PhongTexShaderGL>("phong_tex_mult_lights_vs.glsl", "phong_tex_mult_lights_fs.glsl", 
			"phong_tex_mult_lights_vs_Instanced.glsl");
		break;
	}
	case Playground: {
		shaderPtr = make_shared<PlaygroundShaderGL>("playground_vs.glsl", "playground_fs.glsl", "playground_gs.glsl");
		break;
	}
	case SimpleColor: {
		shaderPtr = make_shared<SimpleColorShaderGL>
			("simpleColor_vs.glsl", "simpleColor_fs.glsl");
		break;
	}
	case SimpleExtrude: {
		shaderPtr = make_shared<SimpleExtrudeShaderGL>
			("simpleExtrude_vs.glsl", "simpleExtrude_fs.glsl");
		break;
	}
	case SimpleLight: {
		shaderPtr = make_shared<SimpleLightShaderGL>
			("simpleLight_vs.glsl", "simpleLight_fs.glsl");
		break;
	}
	case SimpleReflection: {
		shaderPtr = make_shared<SimpleReflectionShaderGL>
			("simpleReflection_vs.glsl", "simpleReflection_fs.glsl");
		break;
	}
	case Screen: {
		shaderPtr = make_shared<ScreenShaderGL>
			("screen_vs.glsl", "screen_fs.glsl");
		break;
	}
	case SkyBox: {
		shaderPtr = make_shared<SkyBoxShaderGL>
			("skybox_vs.glsl", "skybox_fs.glsl");
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