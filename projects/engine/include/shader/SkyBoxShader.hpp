#pragma once

#include <shader/Shader.hpp>
#include <texture/CubeMap.hpp>

class SkyBoxShader : public Shader
{
public:
	virtual ~SkyBoxShader() {}

	virtual void setSkyTexture(CubeMap* sky) = 0;
};