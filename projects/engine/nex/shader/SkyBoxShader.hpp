#pragma once

#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>

class SkyBoxShader : public ShaderConfig
{
public:
	SkyBoxShader() {};
	virtual ~SkyBoxShader() {}
	virtual void setSkyTexture(CubeMap* sky) = 0;
};

class PanoramaSkyBoxShader : public ShaderConfig
{
public:
	PanoramaSkyBoxShader() {};
	virtual ~PanoramaSkyBoxShader() {}
	virtual void setSkyTexture(Texture* tex) = 0;
};


class EquirectangularSkyBoxShader : public ShaderConfig
{
public:
	EquirectangularSkyBoxShader() {};
	virtual ~EquirectangularSkyBoxShader() {}
	virtual void setSkyTexture(Texture* tex) = 0;
};