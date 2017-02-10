#pragma once
#include <drawing/ModelDrawer.hpp>
#include <memory>

class ModelDrawerGL : public ModelDrawer
{
public:
	virtual ~ModelDrawerGL();

	void draw(Vob* vob, Shader* shader, const Shader::TransformData& data) override;
	
	void drawInstanced(Vob* vob, Shader* shader, const Shader::TransformData& data, unsigned amount) override;

	void drawOutlined(Vob* vob, Shader* shader, const Shader::TransformData& data, glm::vec4 borderColor) override;

	void drawWired(Vob* vob, Shader* shader, const Shader::TransformData& data, int lineStrength) override;

	static ModelDrawerGL* get();

protected:
	ModelDrawerGL();

private:
	static std::unique_ptr<ModelDrawerGL> ModelDrawerGL::instance;
};