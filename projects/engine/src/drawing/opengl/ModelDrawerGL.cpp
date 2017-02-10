#include <drawing/opengl/ModelDrawerGL.hpp>
#include <shader/Shader.hpp>
#include <model/Model.hpp>
#include <model/Vob.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.inl>
#include <shader/SimpleColorShader.hpp>
#include <shader/ShaderManager.hpp>
#include <shader/opengl/ShaderManagerGL.hpp>
#include <shader/SimpleExtrudeShader.hpp>
#include <shader/opengl/SimpleExtrudeShaderGL.hpp>
#include <model/opengl/ModelManagerGL.hpp>

using namespace glm;
using namespace std;

unique_ptr<ModelDrawerGL> ModelDrawerGL::instance = make_unique<ModelDrawerGL>(ModelDrawerGL());

ModelDrawerGL::ModelDrawerGL()
{
}

ModelDrawerGL::~ModelDrawerGL()
{
}

void ModelDrawerGL::draw(Vob* vob, Shader* shader, const Shader::TransformData& data)
{
	ShaderManagerGL::get()->validateShader(shader);
	vob->calcTrafo();
	Model* model = ModelManagerGL::get()->getModel(vob->getMeshName());
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	shader->setTransformData(data);
	for (Mesh* mesh : model->getMeshes())
	{
		shader->draw(*mesh);
	}
}

void ModelDrawerGL::drawInstanced(Vob* vob, Shader* shader, const Shader::TransformData& data, unsigned amount)
{
	ShaderManagerGL::get()->validateShader(shader);
	vob->calcTrafo();
	Model* model = ModelManagerGL::get()->getModel(vob->getMeshName());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	shader->setTransformData(data);
	for (Mesh* mesh : model->getMeshes())
	{
		shader->drawInstanced(*mesh, amount);
	}
}

void ModelDrawerGL::drawOutlined(Vob* vob, Shader* shader, const Shader::TransformData& data, vec4 borderColor)
{
	ShaderManagerGL::get()->validateShader(shader);
	glEnable(GL_STENCIL_TEST);

	// always set 1 to the stencil buffer, regardless of the current stencil value
	glStencilFunc(GL_ALWAYS, 1, 0xFF);

	// regardless of fragment will be drawn, update the stencil buffer
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	glStencilMask(0xFF); // enable stencil buffer
	
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);


	//glPolygonMode(GL_FRONT, GL_FILL);
	draw(vob, shader, data);

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

	// only update stencil buffer, if depth and stencil test succeeded
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0x00);
	//glDisable(GL_DEPTH_TEST);


	//draw a slightly scaled up version
	//mat4 scaled = scale(*data.model, vec3(1.1f, 1.1f, 1.1f));
	SimpleExtrudeShader* simpleExtrude = static_cast<SimpleExtrudeShaderGL*>
										(ShaderManagerGL::get()->getShader(SimpleExtrude));
	
	simpleExtrude->setObjectColor(borderColor);
	simpleExtrude->setExtrudeValue(0.05f);
	// use 3 pixel as outline border
	glDepthFunc(GL_ALWAYS);
	draw(vob, simpleExtrude, data);
	//glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// clear stencil buffer state
	glStencilMask(0xFF);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	
	// make stencil buffer read-only again!
	glStencilMask(0x00);
}

void ModelDrawerGL::drawWired(Vob* vob, Shader* shader, const Shader::TransformData& data, int lineStrength)
{
	ShaderManagerGL::get()->validateShader(shader);
	vob->calcTrafo();
	Model* model = ModelManagerGL::get()->getModel(vob->getMeshName());

	glLineWidth(static_cast<float>(lineStrength));
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	shader->setTransformData(data);
	for (Mesh* mesh : model->getMeshes())
	{
		shader->draw(*mesh);
	}
}

ModelDrawerGL* ModelDrawerGL::get()
{
	return instance.get();
}