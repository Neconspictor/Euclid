#pragma once
#include <nex/texture/TextureSamplerData.hpp>

namespace nex
{
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

	enum class WindingOrder
	{
		CLOCKWISE, FIRST = CLOCKWISE,
		COUNTER_CLOCKWISE, LAST = COUNTER_CLOCKWISE
	};

	struct Viewport
	{
		int x;
		int y;
		int width;
		int height;

		Viewport() : x(0), y(0), width(0), height(0) {}
		Viewport(int x, int y, int width, int height) : 
			x(x), y(y), width(width), height(height) {}
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
		BlendFunc source = BlendFunc::ONE;
		BlendFunc destination = BlendFunc::ZERO;
		BlendOperation operation = BlendOperation::ADD;

		static BlendDesc createAlphaTransparency() {
			return { BlendFunc::SOURCE_ALPHA, BlendFunc::ONE_MINUS_SOURCE_ALPHA, BlendOperation::ADD };
		}

		static BlendDesc createMultiplicativeBlending() {
			return {BlendFunc::DESTINATION_COLOR, BlendFunc::ZERO, BlendOperation::ADD};
		}
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

	struct RenderState
	{
		struct Comparator {
			bool operator()(const RenderState& a, const RenderState& b) const;
		};


		bool doDepthTest = true;
		bool doDepthWrite = true;
		CompFunc depthCompare = CompFunc::LESS;
		bool doCullFaces = true;
		PolygonSide cullSide = PolygonSide::BACK;
		WindingOrder windingOrder = WindingOrder::COUNTER_CLOCKWISE;
		bool doBlend = false;
		BlendDesc blendDesc = BlendDesc::createAlphaTransparency();

		bool doShadowCast = true;
		bool doShadowReceive = true;
		FillMode fillMode = FillMode::FILL;
		bool isTool = false;
		unsigned short toolDrawIndex = 0xffff;

		static const RenderState& getDefault() {
			static RenderState state;
				return state;
		}

		static const RenderState& getNoDepthTest() {
			static RenderState state = createNoDepthTest();
			return state;
		}

		static const RenderState& getMultiplicativeBlending() {
			static RenderState state = createMultiplicativeBlending();
			return state;
		}

		bool operator==(const RenderState&);
		bool operator!=(const RenderState&);

	private:
		static RenderState createNoDepthTest()
		{
			RenderState state;
			state.doDepthTest = false;
			state.doDepthWrite = false;
			return state;
		}

		static RenderState createMultiplicativeBlending()
		{
			RenderState state = createNoDepthTest();
			state.doBlend = true;
			state.blendDesc.operation = BlendOperation::ADD;
			state.blendDesc.source = BlendFunc::ZERO;
			state.blendDesc.destination = BlendFunc::SOURCE_COLOR;
			return state;
		}
	};
}