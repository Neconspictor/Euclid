#pragma once
#include <drawing/ModelDrawer.hpp>
#include <memory>

class ModelDrawerGL : public ModelDrawer
{
public:
	virtual ~ModelDrawerGL();

	void draw(const Model& model, Shader* shader, Shader::TransformData data) override;
	
	void drawOutlined(const Model& model, Shader* shader, Shader::TransformData data, glm::vec4 borderColor) override;

	void drawWired(const Model& model, Shader* shader, Shader::TransformData data, int lineStrength) override;

	static ModelDrawerGL* get();

protected:
	ModelDrawerGL();

private:
	static std::unique_ptr<ModelDrawerGL> ModelDrawerGL::instance;
};