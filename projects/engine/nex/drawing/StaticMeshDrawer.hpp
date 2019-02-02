#pragma once

#include <nex/shader/Shader.hpp>

namespace nex
{
	class RenderBackend;
	class Sprite;
	class Vob;
	class SubMesh;
	class StaticMesh;
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
		explicit StaticMeshDrawer(RenderBackend* renderer);

		virtual ~StaticMeshDrawer() = default;

		struct RenderContext
		{
			const glm::mat4* projection;
			const glm::mat4* view;
			nex::Shader* shader;
		};

		using VobRenderCallback = void(const Vob* vob, const RenderContext* context);
		using MeshRenderCallback = void(const nex::SubMesh* mesh, const RenderContext* context);


		static void vobRenderCallbackTest(Vob* vob);

		/**
		 * Draws this scene node and all its children using a specific shader.
		 */
		void draw(SceneNode* root, nex::Shader* shader);

		/**
		 * Draws a sprite onto the screen
		 */
		void draw(Sprite* sprite, nex::TransformShader* shader);

		/**
		 * Draws the specified model with a given shader onto the screen.
		 */
		void draw(StaticMesh* model, nex::Shader* shader);

		//TODO implement
		//void drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data, unsigned amount);

		/**
		 * Draws the specified model onto the screen and outlines
		 * it with a border. TODO implement
		 */
		 //void drawOutlined(Vob* vob, glm::vec4 borderColor);

		void drawWired(StaticMesh* model, nex::Shader* shader, int lineStrength);

	private:
		RenderBackend* renderer;
	};
}