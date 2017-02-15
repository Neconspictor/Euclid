#pragma once
#include <drawing/ModelDrawer.hpp>

class RendererOpenGL;

class ModelDrawerGL : public ModelDrawer
{
public:
	explicit ModelDrawerGL(RendererOpenGL* renderer);

	virtual ~ModelDrawerGL();

	void draw(Sprite* sprite) override;

	void draw(Vob* vob, Shader* shader, const Shader::TransformData& data) override;
	
	void drawInstanced(Vob* vob, Shader* shader, const Shader::TransformData& data, unsigned amount) override;

	void drawOutlined(Vob* vob, Shader* shader, const Shader::TransformData& data, glm::vec4 borderColor) override;

	void drawWired(Vob* vob, Shader* shader, const Shader::TransformData& data, int lineStrength) override;

private:
	RendererOpenGL* renderer;
};
