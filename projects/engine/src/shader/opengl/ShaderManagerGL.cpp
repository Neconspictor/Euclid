#include <shader/opengl/ShaderManagerGL.hpp>
#include <shader/opengl/PlaygroundShaderGL.hpp>
#include <shader/opengl/LampShaderGL.hpp>
#include <shader/opengl/SimpleLightShaderGL.hpp>
#include <sstream>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <exception/ShaderInitException.hpp>

using namespace std;
using namespace platform;

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
	createShader(Playground);
	createShader(SimpleLight);
}

Shader* ShaderManagerGL::createShader(ShaderEnum shaderEnum)
{
	unique_ptr<ShaderGL> shaderPtr;
	switch(shaderEnum)
	{
	case Lamp: {
		shaderPtr = make_unique<LampShaderGL>("vs_light.gls", "fs_light.glsl");
			break;
	}
	case Playground: {
		shaderPtr = make_unique<PlaygroundShaderGL>("vertex.gls", "fragment.glsl");
		break;
	}
	case SimpleLight: {
		shaderPtr = make_unique<SimpleLightShaderGL>
			("vs_simpleLightColor.glsl", "fs_simpleLightColor.glsl");
		break;
	}
	default: {
		stringstream ss;
		ss << BOOST_CURRENT_FUNCTION << " : couldn't create shader for: " << shaderEnum;
		throw ShaderInitException(ss.str());
	}
	}

	Shader* result = shaderPtr.get();
	shaderMap.insert(make_pair(shaderEnum, move(shaderPtr)));
	return result;
}