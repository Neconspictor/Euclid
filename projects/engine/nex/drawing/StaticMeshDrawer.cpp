#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/Scene.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/material/Material.hpp>

void nex::StaticMeshDrawer::draw(const std::vector<RenderCommand>& commands, TransformShader* shader, const RenderState* overwriteState)
{
	for (const auto& command : commands)
	{
		auto* currentShader = shader;
		if (!currentShader)
			currentShader = (TransformShader*) command.material->getShader();

		currentShader->bind();
		currentShader->setModelMatrix(command.worldTrafo, command.prevWorldTrafo);
		currentShader->uploadTransformMatrices();
		StaticMeshDrawer::draw(command.mesh, command.material, overwriteState);
	}
}

void nex::StaticMeshDrawer::draw(const std::multimap<unsigned, RenderCommand>& commands, TransformShader* shader,
	const RenderState* overwriteState)
{
	for (const auto& it : commands)
	{
		const auto& command = it.second;

		auto* currentShader = shader;
		if (!currentShader)
			currentShader = (TransformShader*)command.material->getShader();

		currentShader->bind();
		currentShader->setModelMatrix(command.worldTrafo, command.prevWorldTrafo);
		currentShader->uploadTransformMatrices();
		auto rs = command.material->getRenderState();
		StaticMeshDrawer::draw(command.mesh, command.material, &rs);
	}
}

void nex::StaticMeshDrawer::draw(const std::vector<RenderCommand>& commands, nex::SimpleTransformShader* pass,
	const RenderState* overwriteState)
{
	for (const auto& command : commands)
	{
		pass->bind();
		pass->updateTransformMatrix(command.worldTrafo);
		StaticMeshDrawer::draw(command.mesh, command.material, overwriteState);
	}
}

/*void nex::StaticMeshDrawer::draw(const RenderState& state, const Sprite& sprite, TransformShader* shader)
{
	MeshContainer* spriteModel = MeshManager::get()->getSprite();//getModel(ModelManager::SPRITE_MODEL_NAME, Shaders::Unknown);
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
		thread_local auto* backend = RenderBackend::get();
		backend->drawWithIndices(state, mesh->getTopology(), indexBuffer->getCount(), indexBuffer->getType());
	}
}*/

void nex::StaticMeshDrawer::draw(Mesh* mesh, Material* material, const RenderState* overwriteState)
{
	if (material != nullptr)
	{
		material->upload();
	}

	const auto& vertexArray = mesh->getVertexArray();
	const auto& indexBuffer = mesh->getIndexBuffer();

	vertexArray.bind();
	indexBuffer.bind();

	thread_local auto* backend = RenderBackend::get();
	thread_local RenderState defaultState;


	//set render state
	const RenderState* state = nullptr;
	if (overwriteState != nullptr) {
		state = overwriteState;
	}
	else if (material) {
		state = &material->getRenderState();
	}
	else {
		state = &defaultState;
	}

	backend->drawWithIndices(*state, mesh->getTopology(), indexBuffer.getCount(), indexBuffer.getType());
}

void nex::StaticMeshDrawer::draw(MeshContainer* container, Shader* pass, const RenderState* overwriteState)
{
	pass->bind();
	auto& meshes = container->getMeshes();
	auto& mappings = container->getMappings();

	for (auto& mesh : meshes)
	{
		draw(mesh.get(), mappings.at(mesh.get()));
	}
}

void nex::StaticMeshDrawer::drawFullscreenTriangle(const RenderState& state, Shader* pass)
{
	pass->bind();
	thread_local auto* backend = RenderBackend::get();
	auto* triangle = MeshManager::get()->getNDCFullscreenTriangle();
	triangle->bind();
	backend->drawArray(state, Topology::TRIANGLES, 0, 3);
}

void nex::StaticMeshDrawer::drawFullscreenQuad(const RenderState& state, Shader* pass)
{
	pass->bind();
	thread_local auto* backend = RenderBackend::get();
	auto* quad = MeshManager::get()->getNDCFullscreenPlane();
	quad->bind();
	backend->drawArray(state, Topology::TRIANGLE_STRIP, 0, 4);
}

void nex::StaticMeshDrawer::drawWired(MeshContainer* model, Shader* shader, int lineStrength)
{
	shader->bind();
	thread_local auto* backend = RenderBackend::get();
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
		draw(mesh.get(), mappings.at(mesh.get()));
		renderState.fillMode = backupFillMode;
	}
}
