#pragma once
#include <drawing/ModelDrawer.hpp>

class RendererOpenGL;

class ModelDrawerGL : public ModelDrawer
{
public:
	explicit ModelDrawerGL(RendererOpenGL* renderer);

	virtual ~ModelDrawerGL();

	void draw(Sprite* sprite, Shaders shaderType) override;

	void draw(Vob* vob, Shaders shaderType, const TransformData& data) override;
	
	void drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data, unsigned amount) override;

	void drawOutlined(Vob* vob, Shaders shaderType, const TransformData& data, glm::vec4 borderColor) override;

	void drawWired(Vob* vob, Shaders shaderType, const TransformData& data, int lineStrength) override;

private:
	RendererOpenGL* renderer;
};
