#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/Scene.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/material/Material.hpp>
#include <nex/shader/Technique.hpp>

void nex::StaticMeshDrawer::draw(const std::vector<RenderCommand>& commands, TransformPass* pass, const RenderState* overwriteState)
{
	for (const auto& command : commands)
	{
		auto* currentPass = pass;
		if (!currentPass)
			currentPass = command.material->getTechnique()->getActiveSubMeshPass();
		
		currentPass->setModelMatrix(command.worldTrafo, command.prevWorldTrafo);
		currentPass->uploadTransformMatrices();
		StaticMeshDrawer::draw(command.mesh, command.material, currentPass, overwriteState);
	}
}

void nex::StaticMeshDrawer::draw(const std::vector<RenderCommand>& commands, nex::SimpleTransformPass* pass,
	const RenderState* overwriteState)
{
	for (const auto& command : commands)
	{
		pass->updateTransformMatrix(command.worldTrafo);
		StaticMeshDrawer::draw(command.mesh, command.material, pass, overwriteState);
	}
}

/*void nex::StaticMeshDrawer::draw(const RenderState& state, const Sprite& sprite, TransformPass* shader)
{
	StaticMeshContainer* spriteModel = StaticMeshManager::get()->getSprite();//getModel(ModelManager::SPRITE_MODEL_NAME, Shaders::Unknown);
	//TextureGL* texture = dynamic_cast<TextureGL*>(sprite->getTexture());

	//assert(texture);

	glm::mat4 projection = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	glm::mat4 view = glm::mat4(); // just use identity matrix
	glm::mat4 model = glm::mat4();
	glm::vec2 spriteOrigin(0.5f * sprite.getWidth(), 0.5f * sprite.getHeight());
	glm::vec3 rotation = sprite.getRotation();
	glm::vec3 translation = glm::vec3(sprite.getPosition(), 0.0f);
	glm::vec3 scaling = glm::vec3(sprite.getWidth(), sprite.getHeight(), 1.0f);

	// Matrix application order is scale->rotate->translate
	// But as multiplication is resolved from right to left the order is reversed
	// to translate->rotate->scale

	// first translation
	model = translate(model, translation);

	// rotate around origin
	model = translate(model, glm::vec3(spriteOrigin, 0.0f));
	model = rotate(model, rotation.z, glm::vec3(0, 0, 1)); // rotate around z-axis
	model = rotate(model, rotation.y, glm::vec3(0, 1, 0)); // rotate around y-axis
	model = rotate(model, rotation.x, glm::vec3(1, 0, 0)); // rotate around x-axis
	model = translate(model, glm::vec3(-spriteOrigin, 0.0f));


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
		backend->drawWithIndices(state, mesh->getTopology(), indexBuffer->getCount(), indexBuffer->getType());
	}
}*/

void nex::StaticMeshDrawer::draw(Mesh* mesh, Material* material, Pass* pass, const RenderState* overwriteState)
{
	if (material != nullptr)
	{
		material->upload(pass->getShader());
	}

	const VertexArray* vertexArray = mesh->getVertexArray();
	const IndexBuffer* indexBuffer = mesh->getIndexBuffer();

	vertexArray->bind();
	indexBuffer->bind();

	static auto* backend = RenderBackend::get();


	//set render state
	const RenderState* state = nullptr;
	if (overwriteState != nullptr) state = overwriteState;
	else state = &material->getRenderState();

	backend->drawWithIndices(*state, mesh->getTopology(), indexBuffer->getCount(), indexBuffer->getType());
}

void nex::StaticMeshDrawer::draw(StaticMeshContainer* container, Pass* pass, const RenderState* overwriteState)
{
	auto& meshes = container->getMeshes();
	auto& mappings = container->getMappings();

	for (auto& mesh : meshes)
	{
		draw(mesh.get(), mappings.at(mesh.get()), pass);
	}
}

void nex::StaticMeshDrawer::drawFullscreenTriangle(const RenderState& state, Pass* pass)
{
	static auto* backend = RenderBackend::get();
	auto* triangle = StaticMeshManager::get()->getNDCFullscreenTriangle();
	triangle->bind();
	backend->drawArray(state, Topology::TRIANGLES, 0, 3);
}

void nex::StaticMeshDrawer::drawFullscreenQuad(const RenderState& state, Pass* pass)
{
	static auto* backend = RenderBackend::get();
	auto* quad = StaticMeshManager::get()->getNDCFullscreenPlane();
	quad->bind();
	backend->drawArray(state, Topology::TRIANGLE_STRIP, 0, 4);
}

void nex::StaticMeshDrawer::drawWired(StaticMeshContainer* model, Pass* shader, int lineStrength)
{	
	static auto* backend = RenderBackend::get();
	backend->setLineThickness(static_cast<float>(lineStrength));
	backend->getRasterizer()->setFillMode(FillMode::LINE);

	auto& meshes = model->getMeshes();
	auto& mappings = model->getMappings();

	for (auto& mesh : meshes)
	{
		auto* material = mappings.at(mesh.get());
		auto& renderState = material->getRenderState();
		auto backupFillMode = renderState.fillMode;
		renderState.fillMode = FillMode::LINE;
		draw(mesh.get(), mappings.at(mesh.get()), shader);
		renderState.fillMode = backupFillMode;
	}
}
