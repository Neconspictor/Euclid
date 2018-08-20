#include <nex/opengl/drawing/ModelDrawerGL.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/model/Model.hpp>
#include <nex/model/Vob.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/shader/SimpleColorShader.hpp>
#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <nex/shader/SimpleExtrudeShader.hpp>
#include <nex/opengl/model/ModelManagerGL.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/sprite/Sprite.hpp>

using namespace glm;
using namespace std;

ModelDrawerGL::ModelDrawerGL(RendererOpenGL* renderer): renderer(renderer)
{
	assert(renderer != nullptr);
}

ModelDrawerGL::~ModelDrawerGL()
{
}

void ModelDrawerGL::draw(Sprite * sprite, Shader& shader)
{
	ShaderGL& glShader = dynamic_cast<ShaderGL&>(shader);
	Model* spriteModel = ModelManagerGL::get()->getModel(ModelManager::SPRITE_MODEL_NAME, Shaders::Unknown);
	//TextureGL* texture = dynamic_cast<TextureGL*>(sprite->getTexture());

	//assert(texture);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	mat4 projection = ortho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	mat4 view = mat4(); // just use identity matrix
	mat4 model = mat4();
	vec2 spriteOrigin(0.5f * sprite->getWidth(), 0.5f * sprite->getHeight());
	vec3 rotation = sprite->getRotation();
	vec3 translation = vec3(sprite->getPosition(), 0.0f);
	vec3 scaling = vec3(sprite->getWidth(), sprite->getHeight(), 1.0f);

	// Matrix application order is scale->rotate->translate
	// But as multiplication is resolved from right to left the order is reversed
	// to translate->rotate->scale

	// first translation
	model = translate(model, translation);

	// rotate around origin
	model = translate(model, vec3(spriteOrigin, 0.0f));
	model = rotate(model, rotation.z, vec3(0, 0, 1)); // rotate around z-axis
	model = rotate(model, rotation.y, vec3(0, 1, 0)); // rotate around y-axis
	model = rotate(model, rotation.x, vec3(1, 0, 0)); // rotate around x-axis
	model = translate(model, vec3(-spriteOrigin, 0.0f));


	// finally scale
	model = scale(model, scaling);

	TransformData data = { &projection, &view, &model };
	glShader.setTransformData(data);
	//shader->setOffscreenBuffer(texture->getTexture());
	for (auto& mesh : spriteModel->getMeshes())
	{
		glShader.draw(mesh);
	}
}

void ModelDrawerGL::draw(Sprite* sprite, Shaders shaderType)
{
	Shader* shader = ShaderManagerGL::get()->getShader(shaderType);
	draw(sprite, *shader);
}

void ModelDrawerGL::draw(Vob* vob, Shaders shaderType, const TransformData& data)
{
	Shader* shader = ShaderManagerGL::get()->getShader(shaderType);
	vob->calcTrafo();
	Model* model = ModelManagerGL::get()->getModel(vob->getMeshName(), vob->getMaterialShaderType());
	
	static bool called = false;
	//if (!called) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//	called = true;
	//}
	
	
	shader->setTransformData(data);
	for (auto& mesh : model->getMeshes())
	{
		shader->draw(mesh);
	}
}

void ModelDrawerGL::drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data, unsigned amount)
{
	Shader* shader = ShaderManagerGL::get()->getShader(shaderType);
	vob->calcTrafo();
	Model* model = ModelManagerGL::get()->getModel(vob->getMeshName(), vob->getMaterialShaderType());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	shader->setTransformData(data);
	for (auto& mesh : model->getMeshes())
	{
		shader->drawInstanced(mesh, amount);
	}
}

void ModelDrawerGL::drawOutlined(Vob* vob, Shaders shaderType, const TransformData& data, vec4 borderColor)
{
	glEnable(GL_STENCIL_TEST);

	// always set 1 to the stencil buffer, regardless of the current stencil value
	glStencilFunc(GL_ALWAYS, 1, 0xFF);

	// regardless of fragment will be drawn, update the stencil buffer
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	glStencilMask(0xFF); // enable stencil buffer
	
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);


	//glPolygonMode(GL_FRONT, GL_FILL);
	draw(vob, shaderType, data);

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

	// only update stencil buffer, if depth and stencil test succeeded
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0x00);
	//glDisable(GL_DEPTH_TEST);


	//draw a slightly scaled up version
	//mat4 scaled = scale(*data.model, vec3(1.1f, 1.1f, 1.1f));
	SimpleExtrudeShader* simpleExtrude = static_cast<SimpleExtrudeShader*>
										(ShaderManagerGL::get()->getConfig(Shaders::SimpleExtrude));
	
	simpleExtrude->setObjectColor(borderColor);
	simpleExtrude->setExtrudeValue(0.05f);
	// use 3 pixel as outline border
	glDepthFunc(GL_ALWAYS);
	draw(vob, Shaders::SimpleExtrude, data);
	//glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// clear stencil buffer state
	glStencilMask(0xFF);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	
	// make stencil buffer read-only again!
	glStencilMask(0x00);
}

void ModelDrawerGL::drawWired(Vob* vob, Shaders shaderType, const TransformData& data, int lineStrength)
{
	Shader* shader = ShaderManagerGL::get()->getShader(shaderType);
	vob->calcTrafo();
	Model* model = ModelManagerGL::get()->getModel(vob->getMeshName(), vob->getMaterialShaderType());

	glLineWidth(static_cast<float>(lineStrength));
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	shader->setTransformData(data);
	for (auto& mesh : model->getMeshes())
	{
		shader->draw(mesh);
	}
}