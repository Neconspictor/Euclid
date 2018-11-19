#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>

class ScreenShaderGL : public ShaderGL
{
public:
	ScreenShaderGL();

	virtual ~ScreenShaderGL() = default;

	void useTexture(const TextureGL* texture);

	void setMVP(const glm::mat4& mat);

protected:
	UniformTex mTexture;
	Uniform mTransform;
};