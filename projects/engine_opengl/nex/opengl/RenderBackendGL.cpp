#include <nex/opengl/RenderBackendGL.hpp>

using namespace std;
using namespace nex;
using namespace glm;

namespace nex
{

	TopologyGL translate(Topology topology)
	{
		static TopologyGL table[]
		{
			LINES,
			LINES_ADJACENCY,
			LINE_LOOP,
			LINE_STRIP,
			LINE_STRIP_ADJACENCY,
			PATCHES,
			POINTS,
			TRIANGLES,
			TRIANGLES_ADJACENCY,
			TRIANGLE_FAN,
			TRIANGLE_STRIP,
			TRIANGLE_STRIP_ADJACENCY,
		};

		static const unsigned size = (unsigned)Topology::LAST - (unsigned)Topology::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: Topology and TopologyGL don't match!");

		return table[(unsigned)topology];
	}

	BlendFuncGL translate(BlendFunc func)
	{
		static BlendFuncGL table[]
		{
			ZERO,
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
			ONE_MINUS_CONSTANT_ALPHA,
		};

		static const unsigned size = (unsigned)BlendFunc::LAST - (unsigned)BlendFunc::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: BlendFunc and BlendFuncGL don't match!");

		return table[(unsigned)func];
	}

	BlendOperationGL translate(BlendOperation op)
	{
		static BlendOperationGL table[]
		{
			ADD,
			SUBTRACT,
			REV_SUBTRACT,
			MIN,
			MAX,
		};

		static const unsigned size = (unsigned)BlendOperation::LAST - (unsigned)BlendOperation::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: BlendFunc and BlendFuncGL don't match!");

		return table[(unsigned)op];
	}

	BlendDescGL::BlendDescGL(const BlendDesc& desc) :
		sourceRGB(translate(desc.sourceRGB)),
		destRGB(translate(desc.destRGB)),
		operationRGB(translate(desc.operationRGB)),
		sourceAlpha(translate(desc.sourceAlpha)),
		destAlpha(translate(desc.destAlpha)),
		operationAlpha(translate(desc.operationAlpha))
	{
	}

	BlenderGL::BlenderGL() :
		mGlobalBlendDesc(BlendDesc())
	{
		setState(BlendState());
	}

	void BlenderGL::enableBlend(bool enable)
	{
		mEnableBlend = enable;

		if (enable)
		{
			GLCall(glEnable(GL_BLEND));
		}
		else
		{
			GLCall(glDisable(GL_BLEND));
		}
	}

	void BlenderGL::enableAlphaToCoverage(bool enable)
	{
		mEnableAlphaToCoverage = enable;

		if (enable)
		{
			GLCall(glEnable(GL_SAMPLE_COVERAGE));
		}
		else
		{
			GLCall(glDisable(GL_SAMPLE_COVERAGE));
		}
	}

	void BlenderGL::setSampleConverage(float sampleCoverage, bool invert)
	{
		mSampleCoverage = sampleCoverage;
		mInvertSampleConverage = translate(invert);
		GLCall(glSampleCoverage(mSampleCoverage, mInvertSampleConverage));
	}

	void BlenderGL::setConstantBlendColor(const glm::vec4& color)
	{
		mConstantBlendColor = color;
		GLCall(glBlendColor(color.r, color.g, color.b, color.a));
	}

	void BlenderGL::setGlobalBlendDesc(const BlendDesc& desc)
	{
		mGlobalBlendDesc = desc;
	}

	void BlenderGL::setState(const BlendState& state)
	{
		enableBlend(state.enableBlend);
		enableAlphaToCoverage(state.enableAlphaToCoverage);
		setSampleConverage(state.sampleCoverage, state.invertSampleConverage);
		setConstantBlendColor(state.constantBlendColor);
		setGlobalBlendDesc(state.globalBlendDesc);
	}

	void BlenderGL::setRenderTargetBlending(const RenderTargetBlendDesc& blendDesc)
	{
		RenderTargetBlendDescGL descGL(blendDesc);
		mRenderTargetBlendings[blendDesc.colorAttachIndex] = descGL;

		if (blendDesc.enableBlend)
		{
			GLCall(glEnablei(GL_BLEND, descGL.colorAttachIndex));
			GLCall(glBlendEquationSeparatei(descGL.colorAttachIndex, descGL.blendDesc.operationRGB, descGL.blendDesc.operationAlpha));
			GLCall(glBlendFuncSeparatei(descGL.colorAttachIndex,
				descGL.blendDesc.sourceRGB, descGL.blendDesc.destRGB,
				descGL.blendDesc.sourceAlpha, descGL.blendDesc.destAlpha));

		}
		else
		{
			GLCall(glDisablei(GL_BLEND, descGL.colorAttachIndex));
		}
	}

	RenderTargetBlendDescGL::RenderTargetBlendDescGL(const RenderTargetBlendDesc& desc) :
		enableBlend(translate(desc.enableBlend)),
		colorAttachIndex(desc.colorAttachIndex),
		blendDesc(desc.blendDesc)	
	{
	}

	RasterizerGL::RasterizerGL()
	{
		setState(RasterizerState());
	}

	void RasterizerGL::setFillMode(FillMode fillMode, PolygonSide faceSide)
	{
		const auto fillModeGL = translate(fillMode);
		const auto faceSideGL = translate(faceSide);
		mFillModes[faceSideGL] = fillModeGL;
		GLCall(glPolygonMode(faceSideGL, fillModeGL));
	}

	void RasterizerGL::setCullMode(PolygonSide faceSide)
	{
		cullMode = translate(faceSide);
		GLCall(glCullFace(cullMode));
	}

	void RasterizerGL::setFrontCounterClockwise(bool set)
	{
		mFrontCounterClockwise = set;
		const auto enumGL = set ? GL_CCW : GL_CW;
		GLCall(glFrontFace(enumGL));
	}

	void RasterizerGL::setDepthBias(float slopeScale, float unit, float clamp)
	{
		mDepthBias = unit;
		mSlopeScaledDepthBias = slopeScale;
		mDepthBiasClamp = clamp;

		//TODO use clamp with EXT_polygon_offset_clamp !
		GLCall(glPolygonOffset(slopeScale, unit));
	}

	void RasterizerGL::setState(const RasterizerState& state)
	{
		for (auto& mode : state.fillModes)
		{
			setFillMode(mode.second, mode.first);
		}

		setCullMode(state.cullMode);
		setFrontCounterClockwise(state.frontCounterClockwise);
		setDepthBias(state.slopeScaledDepthBias, state.depthBias, state.depthBiasClamp);
		enableFaceCulling(state.enableFaceCulling);
		enableScissorTest(state.enableScissorTest);
		enableMultisample(state.enableMultisample);
		enableOffsetPolygonFill(state.enableOffsetPolygonFill);
		enableOffsetLine(state.enableOffsetLine);
		enableOffsetPoint(state.enableOffsetPoint);
	}

	void RasterizerGL::enableFaceCulling(bool enable)
	{
		mEnableFaceCulling = enable;
		if (enable)
		{
			GLCall(glEnable(GL_CULL_FACE));
		} else
		{
			GLCall(glDisable(GL_CULL_FACE));
		}
	}

	void RasterizerGL::enableScissorTest(bool enable)
	{
		mEnableScissorTest = enable;

		if (enable)
		{
			GLCall(glEnable(GL_SCISSOR_TEST));
		} else
		{
			GLCall(glDisable(GL_SCISSOR_TEST));
		}
	}

	void RasterizerGL::enableMultisample(bool enable)
	{
		mEnableMultisample = enable;

		if (enable)
		{
			GLCall(glEnable(GL_MULTISAMPLE));
		}
		else
		{
			GLCall(glDisable(GL_MULTISAMPLE));
		}
	}

	void RasterizerGL::enableOffsetPolygonFill(bool enable)
	{
		mEnableOffsetPolygonFill = enable;

		if (enable)
		{
			GLCall(glEnable(GL_POLYGON_OFFSET_FILL));
		}
		else
		{
			GLCall(glDisable(GL_POLYGON_OFFSET_FILL));
		}
	}

	void RasterizerGL::enableOffsetLine(bool enable)
	{
		mEnableOffsetLine = enable;

		if (enable)
		{
			GLCall(glEnable(GL_POLYGON_OFFSET_LINE));
		}
		else
		{
			GLCall(glDisable(GL_POLYGON_OFFSET_LINE));
		}
	}

	void RasterizerGL::enableOffsetPoint(bool enable)
	{
		mEnableOffsetPoint = enable;

		if (enable)
		{
			GLCall(glEnable(GL_POLYGON_OFFSET_POINT));
		}
		else
		{
			GLCall(glDisable(GL_POLYGON_OFFSET_POINT));
		}
	}

	GLuint translate(bool boolean)
	{
		return boolean ? GL_TRUE : GL_FALSE;
	}

	IndexElementTypeGL translate(IndexElementType indexType)
	{
		static IndexElementTypeGL table[]
		{
			BIT_16,
			BIT_32,
		};

		static const unsigned size = (unsigned)IndexElementType::LAST - (unsigned)IndexElementType::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: IndexElementType and IndexElementTypeGL don't match!");

		return table[(unsigned)indexType];
	}

	PolygonSideGL translate(PolygonSide side)
	{
		static PolygonSideGL table[]
		{
			BACK,
			FRONT,
			FRONT_BACK,
		};

		static const unsigned size = (unsigned)PolygonSide::LAST - (unsigned)PolygonSide::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: PolygonSide and PolygonSideGL don't match!");

		return table[(unsigned)side];
	}

	FillModeGL translate(FillMode type)
	{
		static FillModeGL table[]
		{
			FILL,
			LINE,
			POINT,
		};

		static const unsigned size = (unsigned)FillMode::LAST - (unsigned)FillMode::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: PolygonRasterizationType and PolygonRasterizationTypeGL don't match!");

		return table[(unsigned)type];
	}
}