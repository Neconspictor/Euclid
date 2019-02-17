#pragma once
#include <nex/mesh/IndexBuffer.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/opengl/opengl.hpp>

namespace nex {

	enum CompareFunctionGL
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

	enum TopologyGL
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

	enum IndexElementTypeGL {
		BIT_16 = GL_UNSIGNED_SHORT,
		BIT_32 = GL_UNSIGNED_INT,
	};

	enum PolygonSideGL
	{
		BACK = GL_BACK,
		FRONT = GL_FRONT,
		FRONT_BACK = GL_FRONT_AND_BACK,
	};

	enum FillModeGL
	{
		FILL = GL_FILL,
		LINE = GL_LINE,
		POINT = GL_POINT,
	};

	enum BlendFuncGL
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

	enum BlendOperationGL
	{
		ADD = GL_FUNC_ADD, // source + destination
		SUBTRACT = GL_FUNC_SUBTRACT, // source - destination
		REV_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT, // destination - source
		MIN = GL_MIN, // min(source, destination)
		MAX = GL_MAX, // max(source, destination)
	};

	enum PolygonOffsetFillModeGL
	{
		OFFSET_FILL = GL_POLYGON_OFFSET_FILL,
		OFFSET_LINE = GL_POLYGON_OFFSET_LINE,
		OFFSET_POINT = GL_POLYGON_OFFSET_POINT,
	};

	struct BlendDescGL
	{
		BlendFuncGL sourceRGB;
		BlendFuncGL destRGB;
		BlendOperationGL operationRGB;
		BlendFuncGL sourceAlpha;
		BlendFuncGL destAlpha;
		BlendOperationGL operationAlpha;

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


	class BlenderGL : public Blender::Implementation
	{
	public:

		BlenderGL();

		void enableBlend(bool enable);
		void enableAlphaToCoverage(bool enable);
		void setSampleConverage(float sampleCoverage, bool invert);
		void setConstantBlendColor(const glm::vec4& color);
		void setGlobalBlendDesc(const BlendDesc& desc);
		void setState(const BlendState& state);

		void setRenderTargetBlending(const RenderTargetBlendDesc& blendDesc);

	private:
		bool mEnableBlend;
		bool mEnableAlphaToCoverage;
		float mSampleCoverage;
		GLuint mInvertSampleConverage;
		glm::vec4 mConstantBlendColor;
		BlendDescGL mGlobalBlendDesc;

		std::map<unsigned, RenderTargetBlendDescGL> mRenderTargetBlendings;
		
	};

	class DepthBufferGL : public DepthBuffer::Implementation
	{
	public:
		DepthBufferGL();


		void enableDepthBufferWriting(bool enable);
		void enableDepthTest(bool enable);
		void enableDepthClamp(bool enable);

		// depth comparison function being used when depth test is enabled and no sampler is bound
		void setDefaultDepthFunc(CompareFunction depthFunc);

		//specify mapping of depth values from normalized device coordinates to window coordinates
		void setDepthRange(const DepthBuffer::Range& range);

		void setState(const DepthBuffer::State& state);

	private:
		bool mEnableDepthBufferWriting;
		bool mEnableDepthTest;
		bool mEnableDepthClamp;
		CompareFunctionGL mDepthFunc;
		DepthBuffer::Range mDepthRange;
	};


	class RasterizerGL : public Rasterizer::Implementation
	{
	public:
		RasterizerGL();
		virtual ~RasterizerGL() = default;

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

		std::map<PolygonSideGL, FillModeGL> mFillModes;
		PolygonSideGL cullMode;

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

	class StencilTestGL : public StencilTest::Implementation
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


		StencilTestGL();

		void enableStencilTest(bool enable);
		void setCompareFunc(CompareFunction func, int referenceValue, unsigned mask);
		void setOperations(StencilTest::Operation stencilFail, StencilTest::Operation depthFail, StencilTest::Operation depthPass);
		void setState(const StencilTest::State& state);

	private:
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
	CompareFunctionGL translate(nex::CompareFunction);
	IndexElementTypeGL translate(IndexElementType indexType);
	PolygonSideGL translate(PolygonSide side);
	FillModeGL translate(FillMode type);
	TopologyGL translate(Topology topology);

	BlendFuncGL translate(BlendFunc func);
	BlendOperationGL translate(BlendOperation op);

	StencilTestGL::OperationGL translate(StencilTest::Operation op);
}