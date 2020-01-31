#pragma once
#include <memory>
#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/renderer/RenderTypes.hpp>
#include <nex/mesh/MeshTypes.hpp>

namespace nex
{
	struct RenderTargetBlendDesc;
	class GaussianBlur;
	class TextureManager;
	class ShaderManager;
	class CubeDepthMap;
	class EffectLibrary;
	class Sprite;

	class Drawer;
	class ShadingModelFactory;


	enum MemorySync
	{
		MemorySync_ShaderImageAccess = 1 << 0,
		MemorySync_TextureUpdate = 1 << 1,
		MemorySync_TextureFetch = 1 << 2,
		MemorySync_ShaderStorage = 1 << 3
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

	 /**
	  * Configuration class for blending
	  */
	class Blender
	{
	public:
		Blender();

		void enableBlend(bool enable);
		void enableAlphaToCoverage(bool enable);
		void setSampleConverage(float sampleCoverage, bool invert);
		void setConstantBlendColor(const glm::vec4& color);
		void setBlendDesc(const BlendDesc& desc);
		void setState(const BlendState& state);

		void setRenderTargetBlending(const RenderTargetBlendDesc& desc);

	private:
		class Impl;
		std::unique_ptr<Impl> mImpl;
	};

	struct RenderTargetBlendDesc
	{
		bool enableBlend = false;
		unsigned colorAttachIndex = 0;
		BlendDesc blendDesc;
		//unsigned char renderTargetWriteMask; // not supported by opengl
	};


	/**
	 * Configuration class for the depth buffer
	 */
	class DepthBuffer
	{
	public:

		//specify mapping of depth values from normalized device coordinates to window coordinates
		struct Range
		{
			double nearVal;
			double farVal;
		};

		struct State
		{
			bool enableDepthBufferWriting = true;
			bool enableDepthTest = true;
			bool enableDepthClamp = false;
			CompFunc depthFunc = CompFunc::LESS;
			Range depthRange = {0.0, 1.0};
		};

		DepthBuffer();

		void enableDepthBufferWriting(bool enable);
		void enableDepthTest(bool enable);
		void enableDepthClamp(bool enable);

		// depth comparison function being used when depth test is enabled and no sampler is bound
		void setDefaultDepthFunc(CompFunc depthFunc);
		
		//specify mapping of depth values from normalized device coordinates to window coordinates
		void setDepthRange(const Range& range);

		void setState(const State& state);

	private:
		class Impl;
		std::unique_ptr<Impl> mImpl;
	};


	struct RasterizerState
	{
		FillMode fillMode;
		PolygonSide cullMode = PolygonSide::BACK;
		WindingOrder windingOrder = WindingOrder::COUNTER_CLOCKWISE;
		float depthBias = 0.0f;
		float depthBiasClamp = 0.0f;
		float slopeScaledDepthBias = 0.0f;
		//bool enableDepthClipable = false; // not possible in opengl
		bool enableFaceCulling = true;
		bool enableScissorTest = false;
		bool enableMultisample = true;
		// Enable or disables line antialiasing. Note that this option only applies when alpha blending is enabled, 
		// you are drawing lines, and the MultisampleEnable member is FALSE. The default value is FALSE.
		bool enableOffsetPolygonFill = false;
		bool enableOffsetLine = false;
		bool enableOffsetPoint = false;

		RasterizerState()
		{
			fillMode = FillMode::FILL;
		}
	};


	/**
	 * Configuration class for the rasterizer
	 */
	class Rasterizer
	{
	public:
		Rasterizer();

		void setFillMode(FillMode fillMode);
		void setCullMode(PolygonSide faceSide);
		void setWindingOrder(WindingOrder order);
		void setDepthBias(float slopeScale, float unit, float clamp);

		void setState(const RasterizerState& state);
		void enableFaceCulling(bool enable);
		void enableScissorTest(bool enable);
		void enableMultisample(bool enable);
		void enableOffsetPolygonFill(bool enable);
		void enableOffsetLine(bool enable);
		void enableOffsetPoint(bool enable);

	private:
		class Impl;
		std::unique_ptr<Impl> mImpl;
	};


	/**
	  * Configuration class for stencil testing
	  */
	class StencilTest
	{
	public:
		class Impl;

		enum class Operation
		{
			KEEP, FIRST = KEEP,  // The currently stored stencil value is kept.
			ZERO, // The stencil value is set to 0.
			REPLACE, // The stencil value is replaced with the reference value
			INCREMENT, // The stencil value is increased by 1 if it is lower than the maximum value. 
			INCREMENT_WRAP, // Same as INCREMENT, but wraps it back to 0 as soon as the maximum value is exceeded.
			DECREMENT, //  The stencil value is decreased by 1 if it is higher than the minimum value.
			DECREMENT_WRAP, // Same as DECREMENT, but wraps it to the maximum value if it ends up lower than 0.
			INVERT, LAST = INVERT, // Bitwise inverts the current stencil buffer value.
		};

		struct State
		{
			bool enableStencilTest = false;
			CompFunc compareFunc = CompFunc::LESS;
			int compareReferenceValue = 0;
			unsigned compareMask = 0xFF;

			// action to take if the stencil test fails.
			Operation stencilTestFailOperation = Operation::KEEP;

			// action to take if the stencil test passes, but the depth test fails.
			Operation depthTestFailOperation = Operation::KEEP;

			// action to take if both the stencil and the depth test pass.
			Operation depthPassOperation = Operation::KEEP;
		};

		StencilTest();

		void enableStencilTest(bool enable);
		void setCompareFunc(CompFunc func, int referenceValue, unsigned mask);
		void setOperations(Operation stencilFail, Operation depthFail, Operation depthPass);
		void setState(const State& state);

	private:
		std::unique_ptr<Impl> mImpl;
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

		std::unique_ptr < CubeDepthMap> createCubeDepthMap(int width, int height);

		//const TextureData& data = {false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32}
		std::unique_ptr <CubeRenderTarget> createCubeRenderTarget(int width, int height,
			const TextureDesc& data = {
				TexFilter::Linear,
				TexFilter::Linear,
				UVTechnique::ClampToEdge,
				UVTechnique::ClampToEdge,
				UVTechnique::ClampToEdge,
				InternalFormat::RGB32F,
				false });

		std::unique_ptr<RenderTarget2D> create2DRenderTarget(int width, int height,
			const TextureDesc& data = TextureDesc::createImage(
				TexFilter::Linear,
				TexFilter::Linear,
				UVTechnique::ClampToEdge,
				UVTechnique::ClampToEdge,
				UVTechnique::ClampToEdge,
				InternalFormat::RGB32F,
				false
			),
			const TextureDesc& depthData = TextureDesc::createDepth(CompFunc::LESS,
				InternalFormat::DEPTH24_STENCIL8),
			int samples = 1);

		std::unique_ptr<RenderTarget2D> createRenderTarget(int samples = 1);

		//RenderTarget* createVarianceShadowMap(int width, int height);

		/**
		 * Draws primitives directly from the currently bound VertexArray object.
		 * @param primitiveType The topology of the primitives to draw
		 * @param startingIndex Specifies the first vertex to use for drawing
		 * @param indexCount Specifies the number of indices used to draw the primitives beginning from the starting index.
		 */
		void drawArray(const RenderState& state, Topology primitiveType, size_t startingIndex, size_t indexCount);
		void drawArrayInstanced(const RenderState& state, Topology primitiveType, size_t startingIndex, size_t indexCount, size_t instanceCount);

		/**
		 * This functions draws a mesh from the currently bound VertexArray object and the currently bound
		 * IndexBuffer object.
		 */
		void drawWithIndices(const RenderState& state, Topology topology, size_t indexCount, IndexElementType indexType, size_t byteOffset = 0);

		/**
		 * Instanced version of drawWithIndices.
		 */
		void drawWithIndicesInstanced(	size_t instanceCount, 
										const RenderState& state, 
										Topology topology, 
										size_t indexCount, 
										IndexElementType indexType, 
										size_t byteOffset = 0);

		/**
		 * Sends any pending render commands (backend specific) to the GPU.
		 */
		void flushPendingCommands();

		static RenderBackend* get();

		Blender* getBlender();

		RenderTarget2D* getDefaultRenderTarget();

		DepthBuffer* getDepthBuffer();

		// Inherited via RenderBackend
		EffectLibrary* getEffectLibrary();

		/**
		 * Provides the maximum allowed number that can be used for specifiying the number of vertices forming a patch 
		 * when calling a draw command using the Topology::PATCHES topology type.
		 */
		unsigned getMaxPatchVertexCount() const;

		Rasterizer* getRasterizer();

		Sprite* getScreenSprite();

		StencilTest* getStencilTest();

		/**
		* Provides the type of renderer class, this renderer belongs to.
		*/
		RendererType getType() const;

		/**
		* Provides the viewport this renderer is rendering to.
		*/
		const Rectangle& getViewport() const;

		/**
		* Initializes this renderer. After this function call, the renderer is ready to use.
		*/
		void init(const Rectangle& viewport, unsigned msaaSamples);

		/**
		 * Inits the effect library.
		 * NOTE: the TextureManager has to be initialized before initializing the effect library!
		 */
		void initEffectLibrary();

		void resize(int width, int height);

		/**
		* Shuts down this renderer and releases all allocated memory.
		*/
		void release();

		/**
		 * @param flags: a combination of MemorySync flags 
		 */
		void syncMemoryWithGPU(int flags);

		/**
		 * Blocks till all gpu commands are finished.
		 */
		void wait();

		/**
		 * Renders an equirectangular texture (2D) to a cubemap and returns the result;
		 */
		//CubeRenderTarget* renderCubeMap(int width, int height, Texture* equirectangularMap);

		void setBackgroundColor(const glm::vec3& color);

		/**
		 * @param thickness: must be >= 0
		 */
		void setLineThickness(float thickness);

		/**
		 * @param thickness: must be >= 0
		 */
		void setPointThickness(float thickness);

		/**
		 * Sets the number of samples used for msaa
		 */
		void setMSAASamples(unsigned int samples);

		/**
		 * Specifies the number of vertices that form a patch when using a draw call with the Topology::PATCHES topology type.
		 * @param number: Has to be >= 3 and smaller/equal the maximum patch vertex count (retrievable by getMaxPatchVertexCount)
		 */
		void setPatchVertexCount(unsigned number);

		void setScissor(int x, int y, unsigned width, unsigned height);

		/**
		* Sets the viewport size and position.
		*/
		void setViewPort(int x, int y, int width, int height);

	protected:

		void setRenderState(const RenderState& state);

		class Impl;
		std::unique_ptr<Impl> mPimpl;
	};
}