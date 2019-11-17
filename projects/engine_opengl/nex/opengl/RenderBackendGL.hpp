#pragma once
#include <nex/opengl/opengl.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/Sprite.hpp>

namespace nex {

	enum class CompareFunctionGL
	{
		ALWAYS = GL_ALWAYS,
		EQUAL = GL_EQUAL,
		GREATER = GL_GREATER,
		GREATER_EQUAL = GL_GEQUAL,
		LESS = GL_LESS,
		LESS_EQUAL = GL_LEQUAL,
		NEVER = GL_NEVER,
		NOT_EQUAL = GL_NOTEQUAL,
	};

	enum class TopologyGL
	{
		LINES = GL_LINES,
		LINES_ADJACENCY = GL_LINES_ADJACENCY,
		LINE_LOOP = GL_LINE_LOOP,
		LINE_STRIP = GL_LINE_STRIP,
		LINE_STRIP_ADJACENCY = GL_LINE_STRIP_ADJACENCY,
		PATCHES = GL_PATCHES,
		POINTS = GL_POINTS,
		TRIANGLES = GL_TRIANGLES,
		TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY,
		TRIANGLE_FAN = GL_TRIANGLE_FAN,
		TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
		TRIANGLE_STRIP_ADJACENCY = GL_TRIANGLE_STRIP_ADJACENCY,
	};

	enum class IndexElementTypeGL {
		BIT_16 = GL_UNSIGNED_SHORT,
		BIT_32 = GL_UNSIGNED_INT,
	};

	enum class PolygonSideGL
	{
		BACK = GL_BACK,
		FRONT = GL_FRONT,
		FRONT_BACK = GL_FRONT_AND_BACK,
	};

	enum class FillModeGL
	{
		FILL = GL_FILL,
		LINE = GL_LINE,
		POINT = GL_POINT,
	};

	enum class BlendFuncGL
	{
		ZERO = GL_ZERO,
		ONE = GL_ONE,

		SOURCE_COLOR = GL_SRC_COLOR,
		ONE_MINUS_SOURCE_COLOR = GL_ONE_MINUS_SRC_COLOR,
		DESTINATION_COLOR = GL_DST_COLOR,
		ONE_MINUS_DESTINATION_COLOR = GL_ONE_MINUS_DST_COLOR,

		SOURCE_ALPHA = GL_SRC_ALPHA,
		ONE_MINUS_SOURCE_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
		DESTINATION_ALPHA = GL_DST_ALPHA,
		ONE_MINUS_DESTINATION_ALPHA = GL_ONE_MINUS_DST_ALPHA,

		CONSTANT_COLOR = GL_CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
	};

	enum class BlendOperationGL
	{
		ADD = GL_FUNC_ADD, // source + destination
		SUBTRACT = GL_FUNC_SUBTRACT, // source - destination
		REV_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT, // destination - source
		MIN = GL_MIN, // min(source, destination)
		MAX = GL_MAX, // max(source, destination)
	};

	enum class PolygonOffsetFillModeGL
	{
		OFFSET_FILL = GL_POLYGON_OFFSET_FILL,
		OFFSET_LINE = GL_POLYGON_OFFSET_LINE,
		OFFSET_POINT = GL_POLYGON_OFFSET_POINT,
	};

	enum class WindingOrderGL
	{
		CLOCKWISE = GL_CW,
		COUNTER_CLOWCKWISE = GL_CCW,
	};



	class RenderBackend::Impl
	{
	public:

		Impl();

		~Impl();

	private:

		friend RenderBackend;

		//Note: we use public qualifier as this class is private and only used for the RenderBackend!
		glm::vec3 backgroundColor;
		std::unique_ptr<EffectLibrary> mEffectLibrary;
		
		std::unique_ptr<RenderTarget2D> defaultRenderTarget;
		//std::map<unsigned, RenderTargetBlendDesc> mBlendDescs;
		//BlendState mBlendState;

		nex::Logger m_logger{ "RenderBackend" };
		Blender mBlender;
		DepthBuffer mDepthBuffer;
		Rasterizer mRasterizer;
		StencilTest mStencilTest;
		Sprite mScreenSprite;
		Viewport mViewport;
		unsigned int msaaSamples;
	};


	struct BlendDescGL
	{
		BlendFuncGL source;
		BlendFuncGL destination;
		BlendOperationGL operation;

		BlendDescGL(const BlendDesc& desc);
	};

	struct RenderTargetBlendDescGL
	{
		GLuint enableBlend;
		unsigned colorAttachIndex;
		//unsigned char renderTargetWriteMask; // not supported by opengl
		BlendDescGL blendDesc;

		RenderTargetBlendDescGL();
		RenderTargetBlendDescGL(const RenderTargetBlendDesc& desc);
	};

	class Blender::Impl
	{
	public:
		Impl();

	private:
		friend Blender;
		GLboolean mEnableBlend;
		bool mEnableAlphaToCoverage;
		float mSampleCoverage;
		GLuint mInvertSampleConverage;
		glm::vec4 mConstantBlendColor;
		BlendDescGL mBlendDesc;

		std::map<unsigned, RenderTargetBlendDescGL> mRenderTargetBlendings;

	};

	class DepthBuffer::Impl
	{
	public:
		Impl();

	private:
		friend DepthBuffer;

		GLboolean mEnableDepthBufferWriting;
		GLboolean mEnableDepthTest;
		GLboolean mEnableDepthClamp;
		CompareFunctionGL mDepthFunc;
		Range mDepthRange;
	};


	class Rasterizer::Impl
	{
	public:
		Impl();

	private:

		friend Rasterizer;

		struct FillModeCache
		{
			PolygonSideGL side = PolygonSideGL::FRONT;
			FillModeGL mode = FillModeGL::FILL;
		};

		FillModeCache mFillModeCache;
		PolygonSideGL mCullMode;
		WindingOrderGL mWindingOrder;

		bool mFrontCounterClockwise;
		float mDepthBias;
		float mDepthBiasClamp;
		float mSlopeScaledDepthBias;
		//bool enableDepthClipable = false; // not possible in opengl
		bool mEnableFaceCulling;
		bool mEnableScissorTest;
		bool mEnableMultisample;
		// Enable or disables line antialiasing. Note that this option only applies when alpha blending is enabled, 
		// you are drawing lines, and the MultisampleEnable member is FALSE. The default value is FALSE.
		bool mEnableOffsetPolygonFill;
		bool mEnableOffsetLine;
		bool mEnableOffsetPoint;
	};

	class StencilTest::Impl
	{
	public:

		enum OperationGL
		{
			KEEP = GL_KEEP,
			ZERO = GL_ZERO,
			REPLACE = GL_REPLACE,
			INCREMENT = GL_INCR,
			INCREMENT_WRAP = GL_INCR_WRAP,
			DECREMENT = GL_DECR,
			DECREMENT_WRAP = GL_DECR_WRAP,
			INVERT = GL_INVERT,
		};


		Impl();

	private:

		friend StencilTest;

		bool mEnableStencilTest;
		CompareFunctionGL mCompareFunc;
		int mCompareReferenceValue;
		unsigned mCompareMask;

		// action to take if the stencil test fails.
		OperationGL mStencilTestFailOperation;

		// action to take if the stencil test passes, but the depth test fails.
		OperationGL mDepthTestFailOperation;

		// action to take if both the stencil and the depth test pass.
		OperationGL mDepthPassOperation;
	};



	GLuint translate(bool boolean);

	TopologyGL translate(Topology topology);
	BlendFuncGL translate(BlendFunc func);
	BlendOperationGL translate(BlendOperation op);
	StencilTest::Impl::OperationGL translate(StencilTest::Operation op);
	CompareFunctionGL translate(nex::CompFunc compareFunc);
	IndexElementTypeGL translate(IndexElementType indexType);
	PolygonSideGL translate(PolygonSide side);
	FillModeGL translate(FillMode type);
	WindingOrderGL translate(WindingOrder order);
}