#pragma once

#include <nex/shader/Shader.hpp>
#include "nex/renderer/RenderCommand.hpp"

namespace nex
{
	struct RenderState;
	enum class Topology;
	class VertexArray;
	class RenderBackend;
	class Sprite;
	class Vob;
	class Mesh;
	class MeshGroup;
	class SceneNode;

	enum class DrawingTypes
	{
		SOLID = 0,
		INSTANCED,
		OUTLINED,
		WIRED,
	};

	template<class ShaderType>
	struct ShaderOverride {
		ShaderType* default = nullptr;
		ShaderType* rigged = nullptr;
	};

	class MeshDrawer
	{
	public:

		struct RenderContext
		{
			const glm::mat4* projection;
			const glm::mat4* view;
			nex::Shader* pass;
		};

		using VobRenderCallback = void(const Vob* vob, const RenderContext* context);
		using MeshRenderCallback = void(const nex::Mesh* mesh, const RenderContext* context);

		/**
		 * Draws a list of render commands with a specific transform pass and an optional render state (overwrites the render state of the mesh materials);
		 */
		static void draw(const std::vector<RenderCommand>& commands, 
			const Constants& constants,
			const ShaderOverride<nex::Shader>& overrides,
			const RenderState* overwriteState = nullptr);

		static void draw(const std::multimap<unsigned, RenderCommand>& commands,
			const Constants& constants,
			const ShaderOverride<nex::Shader>& overrides,
			const RenderState* overwriteState = nullptr);

		/**
		 * Draws a sprite onto the screen
		 */
		//static void draw(const RenderState& state, const Sprite& sprite, nex::TransformShader* pass);

		/**
		 * Draws the specified model with a given shader onto the screen.
		 * Note: The provided shader isn't bound for performance reasons -> that is the task of the user!
		 */
		static void draw(Shader* shader, const Mesh* mesh, const Material* material, const RenderState* overwriteState = nullptr);

		/**
		 * Draws the specified static mesh container with a given shader onto the screen.
		 * @param shader : The shader to use for rendering the meshes of the mesh container. If null, than the shader 
		 * of the meshes' materials are used.
		 */
		static void draw(MeshGroup* container, Shader* shader, const RenderState* overwriteState = nullptr);

		static void drawFullscreenTriangle(const RenderState& state, Shader* pass);

		static void drawFullscreenQuad(const RenderState& state, Shader* pass);

		//TODO implement
		//void drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data, unsigned amount);

		/**
		 * Draws the specified model onto the screen and outlines
		 * it with a border. TODO implement
		 */
		 //void drawOutlined(Vob* vob, glm::vec4 borderColor);

		static void drawWired(MeshGroup* model, nex::Shader* pass, int lineStrength);

	private:

		static void draw(const RenderCommand& command,
			Shader** lastShaderPtr,
			const Constants& constants,
			const ShaderOverride<nex::Shader>& overrides,
			const RenderState* overwriteState);
	};
}