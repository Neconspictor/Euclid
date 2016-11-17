#include <shader/opengl/ShaderManagerGL.hpp>
#include <shader/opengl/PlaygroundShaderGL.hpp>
#include <shader/opengl/LampShaderGL.hpp>
#include <shader/opengl/SimpleLightShaderGL.hpp>
#include <exception/ShaderNotFoundException.hpp>
#include <sstream>
#include <exception/ShaderInitException.hpp>

using namespace std;

ShaderManagerGL::~ShaderManagerGL()
{
}

Shader* ShaderManagerGL::getShader(const string& shaderName)
{
	auto it = shaderMap.find(shaderName);
	if (it == shaderMap.end())
	{
		stringstream ss; 
		ss << "ShaderManagerGL::getShader:: shader couldn't be matched to s valid shader: " << shaderName;
		throw ShaderNotFoundException(ss.str());
	}

	return it->second.get();
}

void ShaderManagerGL::loadShaders(const string& folder)
{
	//TODO : load shaders from folder!
	auto playground = make_unique<PlaygroundShaderGL>
		("vertex.gls", "fragment.glsl");
	if (!playground || playground.get()->loadingFailed())
	{
		throw ShaderInitException("ShaderManagerGL::loadShaders: PlaygroundShader couldn't be loaded!");
	}
	shaderMap.insert(make_pair("PLAYGROUND", move(playground)));
	
	auto lamp = make_unique<LampShaderGL>
		("vs_light.gls", "fs_light.glsl");
	if (!lamp || lamp.get()->loadingFailed())
	{
		throw ShaderInitException("ShaderManagerGL::loadShaders: LampShader couldn't be loaded!");
	}
	shaderMap.insert(make_pair("LAMP", move(lamp)));


	auto simpleLight = make_unique<SimpleLightShaderGL>
		("vs_simpleLightColor.glsl", "fs_simpleLightColor.glsl");
	if (!simpleLight || simpleLight.get()->loadingFailed())
	{
		throw ShaderInitException("ShaderManagerGL::loadShaders: SimpleLightShader couldn't be loaded!");
	}
	shaderMap.insert(make_pair("SIMPLE_LIGHT", move(simpleLight)));
}