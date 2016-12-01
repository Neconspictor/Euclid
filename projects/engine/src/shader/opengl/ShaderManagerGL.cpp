#include <shader/opengl/ShaderManagerGL.hpp>
#include <shader/opengl/PlaygroundShaderGL.hpp>
#include <shader/opengl/LampShaderGL.hpp>
#include <shader/opengl/SimpleLightShaderGL.hpp>
#include <sstream>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <exception/ShaderInitException.hpp>

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
	createShader(Playground);
	createShader(SimpleLight);
}

ShaderManagerGL* ShaderManagerGL::get()
{
	return instance.get();
}

Shader* ShaderManagerGL::createShader(ShaderEnum shaderEnum)
{
	shared_ptr<ShaderGL> shaderPtr;
	switch(shaderEnum)
	{
	case Lamp: {
		shaderPtr = make_shared<LampShaderGL>("lamp_vs.glsl", "lamp_fs.glsl");
		break;
	}
	case Playground: {
		shaderPtr = make_shared<PlaygroundShaderGL>("playground_vs.glsl", "playground_fs.glsl");
		break;
	}
	case SimpleLight: {
		shaderPtr = make_shared<SimpleLightShaderGL>
			("simpleLight_vs.glsl", "simpleLight_fs.glsl");
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
