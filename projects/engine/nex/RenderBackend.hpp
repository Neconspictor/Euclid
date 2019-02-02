#pragma once
#include <list>
#include <memory>
#include <nex/common/Log.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>

namespace nex
{
	class GaussianBlur;
	class TextureManager;
	class ShaderManager;
	class CubeDepthMap;

	class StaticMeshDrawer;
	class ShadingModelFactory;

	enum class CullingMode
	{
		Front, Back
	};

	enum RenderComponent {
		Color = 1 << 0,
		Depth = 1 << 1,
		Stencil = 1 << 2
	};

	/**
	* A description for the renderer class which category of renderer it belongs to.
	* Typically the renderer type refers to the underlying 3D rendering library, the
	* renderer uses for rendering.
	*/
	enum RendererType : char
	{
		INVALID,   // use this to specify that no renderer will be used 
		OPENGL,
		DIRECTX
	};

	struct Viewport
	{
		int x;
		int y;
		int width;
		int height;

		Viewport() : x(0), y(0), width(0), height(0) {}
	};

	/**
	* Adds a string representation of the renderer type to an output stream.
	*/
	inline std::ostream& operator<< (std::ostream & os, RendererType type)
	{
		switch (type)
		{
		case OPENGL: return os << "OpenGL";
		case DIRECTX: return os << "DirectX";
			// omit default case to trigger compiler warning for missing cases
		};
		return os << static_cast<uint16_t>(type);
	}


	class SMAA;
	class RenderBackend;

	class EffectLibrary {
	public:

		EffectLibrary(RenderBackend* renderer);

		// Inherited via EffectLibrary
		GaussianBlur* getGaussianBlur();

		void release();

	protected:
		std::unique_ptr<GaussianBlur> gaussianBlur;
		RenderBackend* renderer;
	};


	/**
	* A renderer is responsible for visualizing triangle data onto a screen. Commonly, a renderer uses one of the common
	* 3D rendering packages like OpenGL and DirectX. The purpose of this abstract class, is to provide library independent
	* access to 3D Rendering. So all library specific tasks are capsulated in a common interface and applications can use
	* 3D rendering without using specific 3D libraries.
	*/

	/**
	 * A 3D renderer is a renderer specific for 3D content.
	 * A 3D renderer renders so called models (visual content)
	 * and shades them with the use of shaders. A 3D renderer
	 * also understands the concept of textures. As shaders
	 * and textures are strong coupled with the implementation
	 * of a renderer, the RenderBackend interface provides methods
	 * for creating and storing shaders and textures via specialized
	 * manager objects.
	 */
	class RenderBackend
	{
	public:
		RenderBackend();

		virtual ~RenderBackend();

		/**
		* Clears the current scene and begins a new one. A scene is the combination of
		* all, that should be rendered.
		* This function should be called before any rendering is done.
		*/
		void beginScene();

		CubeDepthMap* createCubeDepthMap(int width, int height);

		//const TextureData& data = {false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32}
		CubeRenderTarget* createCubeRenderTarget(int width, int height,
			const TextureData& data = {
				TextureFilter::Linear,
				TextureFilter::Linear,
				TextureUVTechnique::ClampToEdge,
				TextureUVTechnique::ClampToEdge,
				TextureUVTechnique::ClampToEdge,
				ColorSpace::RGB,
				PixelDataType::FLOAT,
				InternFormat::RGB32F,
				false });

		RenderTarget2D* create2DRenderTarget(int width, int height,
			const TextureData& data = {
				TextureFilter::Linear,
				TextureFilter::Linear,
				TextureUVTechnique::ClampToEdge,
				TextureUVTechnique::ClampToEdge,
				TextureUVTechnique::ClampToEdge,
				ColorSpace::RGB,
				PixelDataType::FLOAT,
				InternFormat::RGB32F,
				false
			},
			int samples = 1);

		RenderTarget2D* createRenderTarget(int samples = 1);

		RenderTarget2D* createRenderTargetGL(int width, int height, const TextureData& data, unsigned samples, std::shared_ptr<Texture> depthStencilMap);

		//RenderTarget* createVarianceShadowMap(int width, int height);

		void cullFaces(CullingMode mode);

		void destroyCubeRenderTarget(CubeRenderTarget* target);

		void destroyRenderTarget(RenderTarget2D* target);

		void enableAlphaBlending(bool enable);

		void enableBackfaceDrawing(bool enable);

		/**
		 * Enables / Disables depth mask writing.
		 * models drawn with disabled depth mask will always overwrite
		 * the existing fragments/pixels.
		 */
		void enableDepthWriting(bool enable);

		/**
		 * This functions draws a mesh from the currently bound VertexArray object and the currently bound
		 * IndexBuffer object.
		 */
		void drawWithIndices(Topology topology, unsigned indexCount, IndexElementType indexType);

		/**
		* Finishes the current active scene and sends the resulting data to the GPU.
		*/
		void endScene();

		static RenderBackend* get();

		RenderTarget2D* getDefaultRenderTarget();

		// Inherited via RenderBackend
		EffectLibrary* getEffectLibrary();

		/**
		* Provides the type of renderer class, this renderer belongs to.
		*/
		RendererType getType() const;

		/**
		* Provides the viewport this renderer is rendering to.
		*/
		const Viewport& getViewport() const;

		/**
		* Initializes this renderer. After this function call, the renderer is ready to use.
		*/
		void init();

		void newFrame();

		/**
		* Displays the calculdated scene on the screen. This function has to be called after
		* virtual void Renderer::endSene().
		*/
		void present();

		void resize(int width, int height);

		/**
		* Shuts down this renderer and releases all allocated memory.
		*/
		void release();

		/**
		 * Renders an equirectangular texture (2D) to a cubemap and returns the result;
		 */
		CubeRenderTarget* renderCubeMap(int width, int height, Texture* equirectangularMap);

		void setBackgroundColor(const glm::vec3& color);

		/**
		 * Sets the number of samples used for msaa
		 */
		void setMSAASamples(unsigned int samples);

		/**
		* Sets the viewport size and position.
		*/
		void setViewPort(int x, int y, int width, int height);

	protected:
		glm::vec3 backgroundColor;
		std::list<CubeDepthMap*> cubeDepthMaps;
		std::unique_ptr<EffectLibrary> effectLibrary;
		unsigned int msaaSamples;
		std::list<std::unique_ptr<CubeRenderTarget>> cubeRenderTargets;
		std::list<std::unique_ptr<RenderTarget2D>> mRenderTargets;
		std::unique_ptr<RenderTarget2D> defaultRenderTarget;

	protected:
		nex::Logger m_logger{"RenderBackend"};
		Viewport mViewport;
	};
}
