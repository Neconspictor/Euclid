#include <nex/drawing/MeshDrawer.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/scene/Scene.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/material/Material.hpp>
#include <nex/camera/Camera.hpp>

void nex::MeshDrawer::drawTransform(
	const std::vector<RenderCommand>& commands, 
	const Shader::Constants& constants,
	TransformShader* shader,
	const RenderState* overwriteState)
{

	TransformShader* lastShader = nullptr;

	for (const auto& command : commands)
	{
		draw(command, &lastShader, constants, shader, overwriteState);
	}
}

void nex::MeshDrawer::drawTransform(const std::multimap<unsigned, RenderCommand>& commands,
	const Shader::Constants& constants,
	TransformShader* shader,
	const RenderState* overwriteState)
{
	TransformShader* lastShader = nullptr;

	for (const auto& it : commands)
	{
		draw(it.second, &lastShader, constants, shader, overwriteState);
	}
}

void nex::MeshDrawer::drawSimpleTransform(const std::vector<RenderCommand>& commands,
	const Shader::Constants& constants,
	nex::SimpleTransformShader* shader,
	const RenderState* overwriteState)
{
	SimpleTransformShader* lastShader = nullptr;

	for (const auto& command : commands)
	{
		draw(command, &lastShader, constants, shader, overwriteState);
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

void nex::MeshDrawer::draw(const RenderCommand& command, 
	TransformShader** lastShaderPtr, 
	const Shader::Constants& constants, 
	TransformShader* shader, 
	const RenderState* overwriteState)
{
	auto* lastShader = *lastShaderPtr;
	auto* currentShader = shader;
	if (!currentShader)
		currentShader = (TransformShader*)command.batch->getShader();

	if (lastShader != currentShader) {
		lastShader = currentShader;

		currentShader->bind();

		const auto& camera = *constants.camera;
		currentShader->setViewProjectionMatrices(camera.getProjectionMatrix(),
			camera.getView(), camera.getViewPrev(), camera.getViewProjPrev());

		currentShader->updateConstants(constants);
	}

	currentShader->setModelMatrix(*command.worldTrafo, *command.prevWorldTrafo);
	currentShader->uploadTransformMatrices();

	if (command.isBoneAnimated) {

		ShaderStorageBuffer* buffer = command.boneBuffer;
		auto* data = command.bones;
		buffer->update(data->size() * sizeof(glm::mat4), data->data());
		currentShader->bindBoneTrafoBuffer(buffer);
	}

	for (auto& pair : command.batch->getMeshes()) {
		MeshDrawer::draw(currentShader, pair.first, pair.second, overwriteState);
	}
}


void nex::MeshDrawer::draw(const RenderCommand& command,
	SimpleTransformShader** lastShaderPtr,
	const Shader::Constants& constants,
	SimpleTransformShader* shader,
	const RenderState* overwriteState)
{
	auto* lastShader = *lastShaderPtr;
	auto* currentShader = shader;
	if (!currentShader)
		currentShader = (SimpleTransformShader*)command.batch->getShader();

	if (lastShader != currentShader) {
		lastShader = currentShader;

		currentShader->bind();
		currentShader->updateConstants(constants);
	}

	currentShader->updateTransformMatrix(*command.worldTrafo);

	if (command.isBoneAnimated) {

		ShaderStorageBuffer* buffer = command.boneBuffer;
		auto* data = command.bones;
		buffer->update(data->size() * sizeof(glm::mat4), data->data());
		currentShader->bindBoneTrafoBuffer(buffer);
	}

	for (auto& pair : command.batch->getMeshes()) {
		MeshDrawer::draw(currentShader, pair.first, pair.second, overwriteState);
	}
}
