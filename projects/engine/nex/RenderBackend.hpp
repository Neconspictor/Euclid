#pragma once
#include <list>
#include <memory>
#include <nex/common/Log.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/RenderTarget.hpp>
#include "shader/DepthMapShader.hpp"
#include "../../compute_test/renderer/ComputeTest_Renderer.hpp"

namespace nex
{
	struct RenderTargetBlendDesc;
	class GaussianBlur;
	class TextureManager;
	class ShaderManager;
	class CubeDepthMap;

	class StaticMeshDrawer;
	class ShadingModelFactory;

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

	enum class FillMode
	{
		FILL, FIRST = FILL,
		LINE,
		POINT, LAST = POINT,
	};

	enum class PolygonSide
	{
		BACK, FIRST = BACK,
		FRONT,
		FRONT_BACK, LAST = FRONT_BACK,
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

	enum class BlendFunc
	{
		ZERO, FIRST = ZERO,
		ONE,
		
		SOURCE_COLOR,
		ONE_MINUS_SOURCE_COLOR,
		DESTINATION_COLOR,
		ONE_MINUS_DESTINATION_COLOR,
		
		SOURCE_ALPHA,
		ONE_MINUS_SOURCE_ALPHA,		
		DESTINATION_ALPHA,
		ONE_MINUS_DESTINATION_ALPHA,

		CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA, LAST = ONE_MINUS_CONSTANT_ALPHA,
	};

	enum class BlendOperation
	{
		ADD, FIRST = ADD,// source + destination
		SUBTRACT, // source - destination
		REV_SUBTRACT, // destination - source
		MIN, // min(source, destination)
		MAX, LAST = MAX,// max(source, destination)
	};


	struct BlendDesc
	{
		BlendFunc sourceRGB = BlendFunc::ONE;
		BlendFunc destRGB = BlendFunc::ZERO;
		BlendOperation operationRGB = BlendOperation::ADD;
		BlendFunc sourceAlpha = BlendFunc::ONE;
		BlendFunc destAlpha = BlendFunc::ZERO;
		BlendOperation operationAlpha = BlendOperation::ADD;
	};

	struct BlendState
	{
		bool enableBlend = false;
		bool enableAlphaToCoverage = false;
		float sampleCoverage = 1.0f;
		bool invertSampleConverage = false;
		glm::vec4 constantBlendColor = glm::vec4(0, 0, 0, 0);
		//bool enableIndependentBlend = false; // not possible for opengl

		BlendDesc globalBlendDesc;
	};

	 /**
	  * Configuration class for blending
	  */
	class Blender
	{
	public:

		class Implementation {};

		Blender();

		void enableBlend(bool enable);
		void enableAlphaToCoverage(bool enable);
		void setSampleConverage(float sampleCoverage, bool invert);
		void setConstantBlendColor(const glm::vec4& color);
		void setGlobalBlendDesc(const BlendDesc& desc);
		void setState(const BlendState& state);

		void setRenderTargetBlending(const RenderTargetBlendDesc& blendDesc);

	private:
		std::unique_ptr<Implementation> mImpl;
	};

	struct RenderTargetBlendDesc
	{
		bool enableBlend;
		unsigned colorAttachIndex;
		BlendDesc blendDesc;
		//unsigned char renderTargetWriteMask; // not supported by opengl
	};


	/**
	 * Configuration class for the depth buffer
	 */
	class DepthBuffer
	{
	public:

		class Implementation {};

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
			CompareFunction depthFunc = CompareFunction::LESS;
			Range depthRange = {0.0, 1.0};
		};

		DepthBuffer();

		void enableDepthBufferWriting(bool enable);
		void enableDepthTest(bool enable);
		void enableDepthClamp(bool enable);

		// depth comparison function being used when depth test is enabled and no sampler is bound
		void setDefaultDepthFunc(CompareFunction depthFunc);
		
		//specify mapping of depth values from normalized device coordinates to window coordinates
		void setDepthRange(const Range& range);

		void setState(const State& state);

	private:
		std::unique_ptr<Implementation> mImpl;
	};


	struct RasterizerState
	{
		std::map<PolygonSide, FillMode> fillModes;
		PolygonSide cullMode = PolygonSide::BACK;
		bool frontCounterClockwise = false;
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
			fillModes[PolygonSide::FRONT_BACK] = FillMode::FILL;
		}
	};


	/**
	 * Configuration class for the rasterizer
	 */
	class Rasterizer
	{
	public:

		class Implementation {};

		Rasterizer();

		void setFillMode(FillMode fillMode, PolygonSide faceSide);
		void setCullMode(PolygonSide faceSide);
		void setFrontCounterClockwise(bool set);
		void setDepthBias(float slopeScale, float unit, float clamp);

		void setState(const RasterizerState& state);
		void enableFaceCulling(bool enable);
		void enableScissorTest(bool enable);
		void enableMultisample(bool enable);
		void enableOffsetPolygonFill(bool enable);
		void enableOffsetLine(bool enable);
		void enableOffsetPoint(bool enable);

	private:
		std::unique_ptr<Implementation> mImpl;
	};


	/**
	  * Configuration class for stencil testing
	  */
	class StencilTest
	{
	public:
		class Implementation {};

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
			CompareFunction compareFunc = CompareFunction::LESS;
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
		void setCompareFunc(CompareFunction func, int referenceValue, unsigned mask);
		void setOperations(Operation stencilFail, Operation depthFail, Operation depthPass);
		void setState(const State& state);

	private:
		std::unique_ptr<Implementation> mImpl;
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
			const TextureData& data = TextureData::createImage(
				TextureFilter::Linear,
				TextureFilter::Linear,
				TextureUVTechnique::ClampToEdge,
				TextureUVTechnique::ClampToEdge,
				TextureUVTechnique::ClampToEdge,
				ColorSpace::RGB,
				PixelDataType::FLOAT,
				InternFormat::RGB32F,
				false
			),
			const TextureData& depthData = TextureData::createDepth(CompareFunction::LESS, 
				ColorSpace::DEPTH_STENCIL,
				PixelDataType::UNSIGNED_INT_24_8,
				InternFormat::DEPTH24_STENCIL8),
			int samples = 1);

		RenderTarget2D* createRenderTarget(int samples = 1);

		RenderTarget2D* createRenderTargetGL(int width, int height, const TextureData& data, unsigned samples);

		//RenderTarget* createVarianceShadowMap(int width, int height);

		void destroyCubeRenderTarget(CubeRenderTarget* target);

		void destroyRenderTarget(RenderTarget2D* target);

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

		Blender* getBlender();

		RenderTarget2D* getDefaultRenderTarget();

		DepthBuffer* getDepthBuffer();

		// Inherited via RenderBackend
		EffectLibrary* getEffectLibrary();

		Rasterizer* getRasterizer();

		StencilTest* getStencilTest();

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
		 * @param thickness: must be >= 0
		 */
		void setLineThickness(float thickness);

		/**
		 * Sets the number of samples used for msaa
		 */
		void setMSAASamples(unsigned int samples);

		void setScissor(int x, int y, unsigned width, unsigned height);

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
		//std::map<unsigned, RenderTargetBlendDesc> mBlendDescs;
		//BlendState mBlendState;

		nex::Logger m_logger{"RenderBackend"};
		Blender mBlender;
		DepthBuffer mDepthBuffer;
		Rasterizer mRasterizer;
		StencilTest mStencilTest;
		Viewport mViewport;
	};
}
