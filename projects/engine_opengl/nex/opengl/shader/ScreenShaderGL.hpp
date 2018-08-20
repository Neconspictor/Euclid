#pragma once
#include <nex/shader/ScreenShader.hpp>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

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