#pragma once
#include <nex/drawing/ModelDrawer.hpp>

class RendererOpenGL;
class ShaderGL;

class ModelDrawerGL : public ModelDrawer
{
public:
	explicit ModelDrawerGL(RendererOpenGL* renderer);

	virtual ~ModelDrawerGL();

	void draw(Sprite* sprite, Shaders shaderType) override;

	virtual void draw(Sprite* sprite, Shader& shader) override;

	void draw(Vob* vob, Shaders shaderType, const TransformData& data) override;

	void draw(Model* vob, Shader* shader);
	
	void drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data, unsigned amount) override;

	void drawOutlined(Vob* vob, Shaders shaderType, const TransformData& data, glm::vec4 borderColor) override;

	void drawWired(Vob* vob, Shaders shaderType, const TransformData& data, int lineStrength) override;

private:
	RendererOpenGL* renderer;
};
