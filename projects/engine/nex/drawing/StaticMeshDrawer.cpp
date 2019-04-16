#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/mesh/Vob.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/texture/Sprite.hpp>
#include <nex/Scene.hpp>
#include <nex/RenderBackend.hpp>

//TODO get it from repo history again
//#include "nex/opengl/shader/SimpleExtrudeShaderGL.hpp"

using namespace glm;
using namespace std;
using namespace nex;

void nex::StaticMeshDrawer::draw(SceneNode* root, Pass* shader)
{
	for (auto it = root->mChilds.begin(); it != root->mChilds.end(); ++it)
		draw(*it, shader);

	if (!root->mVob) return;

	if (root->mDrawingType == DrawingTypes::SOLID)
	{
		shader->onModelMatrixUpdate(root->mVob->getTrafo());
		draw(root->mVob->getModel(), shader);
	}
	else if (root->mDrawingType == DrawingTypes::INSTANCED)
	{
		nex::Logger("ModelDrawerGL")(nex::Warning) << "Instanced Drawing type currently not supported";
		//drawer->drawInstanced(vob, type, data, instanceCount);
	}
}

void nex::StaticMeshDrawer::draw(const Sprite& sprite, TransformPass* shader)
{
	StaticMeshContainer* spriteModel = StaticMeshManager::get()->getSprite();//getModel(ModelManager::SPRITE_MODEL_NAME, Shaders::Unknown);
	//TextureGL* texture = dynamic_cast<TextureGL*>(sprite->getTexture());

	//assert(texture);

	mat4 projection = ortho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	mat4 view = mat4(); // just use identity matrix
	mat4 model = mat4();
	vec2 spriteOrigin(0.5f * sprite.getWidth(), 0.5f * sprite.getHeight());
	vec3 rotation = sprite.getRotation();
	vec3 translation = vec3(sprite.getPosition(), 0.0f);
	vec3 scaling = vec3(sprite.getWidth(), sprite.getHeight(), 1.0f);

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

	shader->bind();

	const TransformData data = { &projection, &view, &model };
	shader->onTransformUpdate(data);

	for (auto& mesh : spriteModel->getMeshes())
	{
		const VertexArray* vertexArray = mesh->getVertexArray();
		const IndexBuffer* indexBuffer = mesh->getIndexBuffer();

		vertexArray->bind();
		indexBuffer->bind();
		static auto* backend = RenderBackend::get();
		backend->drawWithIndices(mesh->getTopology(), indexBuffer->getCount(), indexBuffer->getType());

		//indexBuffer->unbind();
		//vertexArray->unbind();
	}
}

void nex::StaticMeshDrawer::draw(StaticMeshContainer* model, Pass* pass)
{
	//TODO
	//shader->bind();
	//shader->setTransformData(data);
	for (auto& mesh : model->getMeshes())
	{
		auto* material = mesh.get()->getMaterial();
		if (material != nullptr)
		{
			material->upload(pass->getShader());
		}
		pass->onMaterialUpdate(mesh.get()->getMaterial());

		const VertexArray* vertexArray = mesh->getVertexArray();
		const IndexBuffer* indexBuffer = mesh->getIndexBuffer();

		vertexArray->bind();
		indexBuffer->bind();

		static auto* backend = RenderBackend::get();

		backend->drawWithIndices(mesh->getTopology(), indexBuffer->getCount(), indexBuffer->getType());

		//indexBuffer->unbind();
		//vertexArray->unbind();
	}
}

/*void ModelDrawerGL::drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data, unsigned amount)
{
	ShaderGL* shader = ShaderManagerGL::get()->getShader(shaderType);
	//vob->calcTrafo();
	ModelGL* model = vob->getModel();//ModelManagerGL::get()->getModel(vob->getMeshName(), vob->getMaterialShaderType());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	shader->setTransformData(data);
	for (auto& mesh : model->getMeshes())
	{
		shader->drawInstanced(mesh, amount);
	}
}*/

//TODO
/*
void ModelDrawerGL::drawOutlined(Vob* vob, ShaderType shaderType, const TransformData& data, vec4 borderColor)
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
	SimpleExtrudeShaderGL* simpleExtrude = static_cast<SimpleExtrudeShaderGL*>
										(ShaderManagerGL::get()->getShader(ShaderType::SimpleExtrude));
	
	simpleExtrude->setObjectColor(borderColor);
	simpleExtrude->setExtrudeValue(0.05f);
	// use 3 pixel as outline border
	glDepthFunc(GL_ALWAYS);
	draw(vob, ShaderType::SimpleExtrude, data);
	//glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// clear stencil buffer state
	glStencilMask(0xFF);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	
	// make stencil buffer read-only again!
	glStencilMask(0x00);
}*/

void nex::StaticMeshDrawer::drawWired(StaticMeshContainer* model, Pass* shader, int lineStrength)
{	
	//TODO
	//vob->calcTrafo();

	static auto* backend = RenderBackend::get();
	backend->setLineThickness(static_cast<float>(lineStrength));
	backend->getRasterizer()->setFillMode(FillMode::LINE, PolygonSide::FRONT_BACK);

	draw(model, shader);
}