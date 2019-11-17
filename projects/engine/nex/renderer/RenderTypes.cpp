#include <nex/renderer/RenderTypes.hpp>
#include <cstring>

bool nex::RenderState::operator==(const RenderState& o)
{
	int cmp = std::memcmp(this, &o, sizeof(RenderState));
	return cmp == 0;
}

bool nex::RenderState::operator!=(const RenderState& o)
{
	return !operator==(o);
}

bool nex::RenderState::Comparator::operator()(const nex::RenderState& a, const nex::RenderState& b) const
{
	int cmp = std::memcmp(&a, &b, sizeof(RenderState));
	return cmp < 0;
}