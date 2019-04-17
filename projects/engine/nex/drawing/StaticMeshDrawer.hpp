#pragma once

#include <nex/shader/Pass.hpp>

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
		 * Draws this scene node and all its children using a specific pass.
		 */
		static void draw(SceneNode* root, nex::TransformPass* pass);

		/**
		 * Draws a sprite onto the screen
		 */
		//static void draw(const RenderState& state, const Sprite& sprite, nex::TransformPass* pass);

		/**
		 * Draws the specified model with a given shader onto the screen.
		 */
		static void draw(Mesh* mesh, Material* material, Pass* pass);

		/**
		 * Draws the specified static mesh container with a given shader onto the screen.
		 */
		static void draw(StaticMeshContainer* container, Pass* pass);

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