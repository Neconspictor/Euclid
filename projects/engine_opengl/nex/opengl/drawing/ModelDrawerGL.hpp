#pragma once

#include <nex/opengl/shader/ShaderGL.hpp>

namespace nex
{
	class RendererOpenGL;
	class Sprite;
	class Vob;
	class MeshGL;
	class ModelGL;
	class SceneNode;

	enum class DrawingTypes
	{
		SOLID = 0,
		INSTANCED,
		OUTLINED,
		WIRED,
	};

	class ModelDrawerGL
	{
	public:
		explicit ModelDrawerGL(RendererOpenGL* renderer);

		virtual ~ModelDrawerGL() = default;

		struct RenderContext
		{
			const glm::mat4* projection;
			const glm::mat4* view;
			nex::Shader* shader;
		};

		using VobRenderCallback = void(const Vob* vob, const RenderContext* context);
		using MeshRenderCallback = void(const nex::MeshGL* mesh, const RenderContext* context);


		static void vobRenderCallbackTest(Vob* vob);

		/**
		 * Draws this scene node and all its children using a specific shader.
		 */
		void draw(SceneNode* root, nex::Shader* shader);

		/**
		 * Draws a sprite onto the screen
		 */
		void draw(Sprite* sprite, nex::TransformShaderGL* shader);

		/**
		 * Draws the specified model with a given shader onto the screen.
		 */
		void draw(ModelGL* model, nex::Shader* shader);

		//TODO implement
		//void drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data, unsigned amount);

		/**
		 * Draws the specified model onto the screen and outlines
		 * it with a border. TODO implement
		 */
		 //void drawOutlined(Vob* vob, glm::vec4 borderColor);

		void drawWired(ModelGL* model, nex::Shader* shader, int lineStrength);

	private:
		RendererOpenGL* renderer;
	};
}