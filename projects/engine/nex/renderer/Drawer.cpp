#include <nex/renderer/Drawer.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/material/Material.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/renderer/RenderCommand.hpp>

void nex::Drawer::draw(
	const std::vector<RenderCommand>& commands, 
	const Constants& constants,
	const ShaderOverride<nex::Shader>& overrides,
	const RenderState* overwriteState)
{

	Shader* lastShader = nullptr;

	for (const auto& command : commands)
	{
		command.renderFunc(command, &lastShader, constants, overrides, overwriteState);
		//drawCommand(command, &lastShader, constants, overrides, overwriteState);
	}
}

void nex::Drawer::draw(const std::multimap<unsigned, RenderCommand>& commands,
	const Constants& constants,
	const ShaderOverride<nex::Shader>& overrides,
	const RenderState* overwriteState)
{
	Shader* lastShader = nullptr;

	for (const auto& it : commands)
	{
		const auto& command = it.second;
		command.renderFunc(command, &lastShader, constants, overrides, overwriteState);
		//drawCommand(command, &lastShader, constants, overrides, overwriteState);
	}
}

void nex::Drawer::draw(Shader* shader, const Mesh* mesh, const Material* material, const RenderState* overwriteState)
{
	if (material != nullptr)
	{
		shader->updateMaterial(*material);
	}

	auto useIndexBuffer = mesh->getUseIndexBuffer();

	const auto& vertexArray = mesh->getVertexArray();
	const auto& indexBuffer = mesh->getIndexBuffer();

	vertexArray.bind();

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

	if (useIndexBuffer) {
		indexBuffer.bind();
		backend->drawWithIndices(*state, mesh->getTopology(), indexBuffer.getCount(), indexBuffer.getType());
	}
	else {
		backend->drawArray(*state, mesh->getTopology(), mesh->getArrayOffset(), mesh->getVertexCount());
	}
}

void nex::Drawer::draw(MeshGroup* container, Shader* shader, const RenderState* overwriteState)
{
	if (shader) {
		shader->bind();
	}
		
	auto& meshes = container->getEntries();
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

void nex::Drawer::drawFullscreenTriangle(const RenderState& state, Shader* pass)
{
	pass->bind();
	thread_local auto* backend = RenderBackend::get();
	auto* triangle = MeshManager::get()->getNDCFullscreenTriangle();
	triangle->bind();
	backend->drawArray(state, Topology::TRIANGLES, 0, 3);
}

void nex::Drawer::drawFullscreenQuad(const RenderState& state, Shader* pass)
{
	pass->bind();
	thread_local auto* backend = RenderBackend::get();
	auto* quad = MeshManager::get()->getNDCFullscreenPlane();
	quad->bind();
	backend->drawArray(state, Topology::TRIANGLE_STRIP, 0, 4);
}

void nex::Drawer::drawWired(MeshGroup* model, Shader* shader, int lineStrength)
{
	shader->bind();
	thread_local auto* backend = RenderBackend::get();
	backend->setLineThickness(static_cast<float>(lineStrength));
	backend->getRasterizer()->setFillMode(FillMode::LINE);

	auto& meshes = model->getEntries();
	auto& mappings = model->getMappings();

	for (auto& mesh : meshes)
	{
		auto* material = mappings.at(mesh.get());
		auto& renderState = material->getRenderState();
		auto backupFillMode = renderState.fillMode;
		renderState.fillMode = FillMode::LINE;
		draw(shader, mesh.get(), material);
		renderState.fillMode = backupFillMode;
	}
}

void nex::Drawer::drawCommand(const RenderCommand& command, 
	Shader** lastShaderPtr, 
	const Constants& constants, 
	const ShaderOverride<nex::Shader>& overrides,
	const RenderState* overwriteState)
{
	auto* lastShader = *lastShaderPtr;
	auto* currentShader = command.isBoneAnimated ? overrides.rigged : overrides.default;

	if (!currentShader)
		currentShader = command.batch->getShader();

	if (lastShader != currentShader) {
		*lastShaderPtr = currentShader;

		currentShader->bind();
		currentShader->updateConstants(constants);
	}

	currentShader->updateInstance(*command.worldTrafo, *command.prevWorldTrafo);

	if (command.isBoneAnimated) {

		ShaderStorageBuffer* buffer = command.boneBuffer;
		auto* data = command.bones;
		buffer->update(data->size() * sizeof(glm::mat4), data->data());
		currentShader->bindBoneTrafoBuffer(buffer);
	}

	for (auto& pair : command.batch->getEntries()) {
		Drawer::draw(currentShader, pair.first, pair.second, overwriteState);
	}
}