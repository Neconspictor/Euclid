#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class ScreenShaderGL : public ShaderConfigGL
{
public:
	ScreenShaderGL();

	virtual ~ScreenShaderGL();

	void update(const MeshGL& mesh, const TransformData& data) override;
	void useTexture(TextureGL* texture);

protected:
	TextureGL* texture;
	glm::mat4 transform;
};