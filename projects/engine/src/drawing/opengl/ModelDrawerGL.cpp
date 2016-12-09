#include <drawing/opengl/ModelDrawerGL.hpp>
#include <shader/Shader.hpp>
#include <model/Model.hpp>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.inl>
#include <shader/SimpleColorShader.hpp>
#include <shader/ShaderManager.hpp>
#include <shader/opengl/ShaderManagerGL.hpp>

using namespace glm;
using namespace std;

unique_ptr<ModelDrawerGL> ModelDrawerGL::instance = make_unique<ModelDrawerGL>(ModelDrawerGL());

ModelDrawerGL::ModelDrawerGL()
{
}

ModelDrawerGL::~ModelDrawerGL()
{
}

void ModelDrawerGL::draw(const Model& model, Shader* shader, Shader::TransformData data)
{
	shader->setTransformData(data);
	for (Mesh* mesh : model.getMeshes())
	{
		shader->draw(*mesh);
	}
}

void ModelDrawerGL::drawOutlined(const Model& model, Shader* shader, Shader::TransformData data, vec3 borderColor)
{
	//glClear(GL_STENCIL_BUFFER_BIT);

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF); // enable stencil buffer
	draw(model, shader, data);

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	//draw a slightly scaled up version
	mat4 scaled = scale(*data.model, vec3(1.1f, 1.1f, 1.1f));
	SimpleColorShader* simpleColor = static_cast<SimpleColorShader*>
										(ShaderManagerGL::get()->getShader(SimpleColor));
	
	simpleColor->setObjectColor(borderColor);
	draw(model, simpleColor, {data.projection, data.view, &scaled});
	glEnable(GL_DEPTH_TEST);
	glStencilMask(0xFF);
}

ModelDrawerGL* ModelDrawerGL::get()
{
	return instance.get();
}