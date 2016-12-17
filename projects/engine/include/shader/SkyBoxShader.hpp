#ifndef ENGINE_SHADER_SKYBOX_SHADER_HPP
#define ENGINE_SHADER_SKYBOX_SHADER_HPP
#include <shader/Shader.hpp>
#include <texture/CubeMap.hpp>

class SkyBoxShader : public Shader
{
public:
	virtual ~SkyBoxShader() {}

	virtual void setSkyTexture(CubeMap* sky) = 0;
};

#endif