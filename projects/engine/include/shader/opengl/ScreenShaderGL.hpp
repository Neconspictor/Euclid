#pragma once
#include <shader/ScreenShader.hpp>
#include <shader/opengl/ShaderGL.hpp>
#include <texture/opengl/TextureGL.hpp>

class ScreenShaderGL : public ScreenShader, public ShaderConfigGL
{
public:
	ScreenShaderGL();

	virtual ~ScreenShaderGL();

	virtual void update(const MeshGL& mesh, const TransformData& data) override;
	virtual void useTexture(Texture* texture) override;

protected:
	TextureGL* texture;
	glm::mat4 transform;
};