#include <nex/drawing/MeshDrawer.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/scene/Scene.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/material/Material.hpp>

void nex::MeshDrawer::draw(const std::vector<RenderCommand>& commands, TransformShader* shader, const RenderState* overwriteState)
{
	for (const auto& command : commands)
	{
		auto* currentShader = shader;
		if (!currentShader)
			currentShader = (TransformShader*) command.batch->getShader();

		currentShader->bind();
		currentShader->setModelMatrix(*command.worldTrafo, *command.prevWorldTrafo);
		currentShader->uploadTransformMatrices();

		for (auto& pair : command.batch->getMeshes()) {
			MeshDrawer::draw(currentShader, pair.first, pair.second, overwriteState);
		}

		
	}
}
//TODO fix it???
void nex::MeshDrawer::draw(const std::multimap<unsigned, RenderCommand>& commands, TransformShader* shader,
	const RenderState* overwriteState)
{
	for (const auto& it : commands)
	{
		const auto& command = it.second;

		auto* currentShader = shader;
		if (!currentShader)
			currentShader = (TransformShader*)command.batch->getShader();

		currentShader->bind();
		currentShader->setModelMatrix(*command.worldTrafo, *command.prevWorldTrafo);
		currentShader->uploadTransformMatrices();
		auto rs = command.batch->getState();

		for (auto& pair : command.batch->getMeshes()) {
			MeshDrawer::draw(currentShader, pair.first, pair.second, &rs);
		}
	}
}

void nex::MeshDrawer::draw(const std::vector<RenderCommand>& commands, nex::SimpleTransformShader* pass,
	const RenderState* overwriteState)
{
	for (const auto& command : commands)
	{
		pass->bind();
		pass->updateTransformMatrix(*command.worldTrafo);

		for (auto& pair : command.batch->getMeshes()) {
			MeshDrawer::draw(pass, pair.first, pair.second, overwriteState);
		}
	}
}

void nex::MeshDrawer::draw(Shader* shader, const Mesh* mesh, const Material* material, const RenderState* overwriteState)
{
	if (material != nullptr)
	{
		shader->upload(*material);
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

void nex::MeshDrawer::draw(MeshGroup* container, Shader* shader, const RenderState* overwriteState)
{
	if (shader) {
		shader->bind();
	}
		
	auto& meshes = container->getMeshes();
	auto& mappings = container->getMappings();

	for (auto& mesh : meshes)
	{
		auto* material = mappings.at(mesh.get());
		auto* currentShader = shader;
		if (!currentShader) {
			currentShader = material->getShader();
		}
		draw(currentShader, mesh.get(), material);
	}
}

void nex::MeshDrawer::drawFullscreenTriangle(const RenderState& state, Shader* pass)
{
	pass->bind();
	thread_local auto* backend = RenderBackend::get();
	auto* triangle = MeshManager::get()->getNDCFullscreenTriangle();
	triangle->bind();
	backend->drawArray(state, Topology::TRIANGLES, 0, 3);
}

void nex::MeshDrawer::drawFullscreenQuad(const RenderState& state, Shader* pass)
{
	pass->bind();
	thread_local auto* backend = RenderBackend::get();
	auto* quad = MeshManager::get()->getNDCFullscreenPlane();
	quad->bind();
	backend->drawArray(state, Topology::TRIANGLE_STRIP, 0, 4);
}

void nex::MeshDrawer::drawWired(MeshGroup* model, Shader* shader, int lineStrength)
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
		draw(shader, mesh.get(), mappings.at(mesh.get()));
		renderState.fillMode = backupFillMode;
	}
}