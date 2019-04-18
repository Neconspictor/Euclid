#pragma once

#include <nex/shader/Pass.hpp>
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
	class StaticMeshContainer;
	class SceneNode;

	enum class DrawingTypes
	{
		SOLID = 0,
		INSTANCED,
		OUTLINED,
		WIRED,
	};

	class StaticMeshDrawer
	{
	public:

		struct RenderContext
		{
			const glm::mat4* projection;
			const glm::mat4* view;
			nex::Pass* pass;
		};

		using VobRenderCallback = void(const Vob* vob, const RenderContext* context);
		using MeshRenderCallback = void(const nex::Mesh* mesh, const RenderContext* context);

		/**
		 * Draws a list of render commands with a specific transform pass and an optional render state (overwrites the render state of the mesh materials);
		 */
		static void draw(const std::vector<RenderCommand>& commands, nex::TransformPass* pass, const RenderState* overwriteState = nullptr);

		/**
		 * Draws a sprite onto the screen
		 */
		//static void draw(const RenderState& state, const Sprite& sprite, nex::TransformPass* pass);

		/**
		 * Draws the specified model with a given shader onto the screen.
		 */
		static void draw(Mesh* mesh, Material* material, Pass* pass, const RenderState* overwriteState = nullptr);

		/**
		 * Draws the specified static mesh container with a given shader onto the screen.
		 */
		static void draw(StaticMeshContainer* container, Pass* pass, const RenderState* overwriteState = nullptr);

		static void drawFullscreenTriangle(const RenderState& state, Pass* pass);

		static void drawFullscreenQuad(const RenderState& state, Pass* pass);

		//TODO implement
		//void drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data, unsigned amount);

		/**
		 * Draws the specified model onto the screen and outlines
		 * it with a border. TODO implement
		 */
		 //void drawOutlined(Vob* vob, glm::vec4 borderColor);

		static void drawWired(StaticMeshContainer* model, nex::Pass* pass, int lineStrength);
	};
}
