#include <nex/renderer/RenderTypes.hpp>
#include <cstring>

bool nex::RenderState::operator<(const RenderState& o) const
{
	if (doDepthTest != o.doDepthTest) return doDepthTest < o.doDepthTest;
	if (doCullFaces != o.doCullFaces) return doCullFaces < o.doCullFaces;
	if (doBlend != o.doBlend) return doBlend < o.doBlend;
	if (doDepthWrite != o.doDepthWrite) return doDepthWrite < o.doDepthWrite;
	if (doShadowCast != o.doShadowCast) return doShadowCast < o.doShadowCast;
	if (doShadowReceive != o.doShadowReceive) return doShadowReceive < o.doShadowReceive;
	if (fillMode != o.fillMode) return fillMode < o.fillMode;
	if (blendDesc != o.blendDesc) return blendDesc < o.blendDesc;
	if (cullSide != o.cullSide) return cullSide < o.cullSide;
	if (isTool != o.isTool) return isTool < o.isTool;
	if (toolDrawIndex != o.toolDrawIndex) return toolDrawIndex < o.toolDrawIndex;
	if (windingOrder != o.windingOrder) return windingOrder < o.windingOrder;

	return false;
}

bool nex::RenderState::operator!=(const RenderState& o) const
{
	return (*this < o) || (o < *this);
}

bool nex::RenderState::Comparator::operator()(const nex::RenderState& a, const nex::RenderState& b) const
{
	return a < b;
}

bool nex::BlendDesc::operator<(const BlendDesc& o) const
{
	if (o.source != source) return o.source < source;
	if (o.destination != destination) return o.destination < destination;
	if (o.operation != operation) return o.operation < operation;
	return false;
}

bool nex::BlendDesc::operator!=(const BlendDesc& o) const
{
	return (*this < o) || (o < *this);
}