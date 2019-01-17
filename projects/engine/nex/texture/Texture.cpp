#include <nex/texture/Texture.hpp>
#include <cassert>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

mat4 nex::CubeMap::rightSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMap::leftSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMap::topSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
mat4 nex::CubeMap::bottomSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
mat4 nex::CubeMap::frontSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 nex::CubeMap::backSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f));


const mat4& nex::CubeMap::getViewLookAtMatrixRH(Side side)
{
	switch (side) {
	case Side::POSITIVE_X:
		return rightSide;
	case Side::NEGATIVE_X:
		return leftSide;
	case Side::POSITIVE_Y:
		return topSide;
	case Side::NEGATIVE_Y:
		return bottomSide;
	case Side::NEGATIVE_Z:
		return frontSide;
	case Side::POSITIVE_Z:
		return backSide;
	default:
		throw_with_trace(std::runtime_error("No mapping defined for CubeMap side!"));
	}

	// won't be reached
	return rightSide;
}

unsigned nex::getComponents(const ColorSpace colorSpace)
{
	static unsigned const table[]
	{
		1,
		1,
		2,
		3,
		4,
		3,
		4,
	};

	static const unsigned size = (unsigned)ColorSpace::LAST - (unsigned)ColorSpace::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: target descriptor list doesn't match number of supported targets");

	return table[(unsigned)colorSpace];
}

bool nex::isCubeTarget(TextureTarget target)
{
	auto first  = static_cast<unsigned>(TextureTarget::CUBE_POSITIVE_X);
	auto last = static_cast<unsigned>(TextureTarget::CUBE_NEGATIVE_Z);
	auto current = static_cast<unsigned>(target);
	return first <= current && last >= current;
}

nex::TextureImpl* nex::Texture::getImpl() const
{
	return mImpl.get();
}

void nex::Texture::resize(unsigned width, unsigned height)
{
	mImpl->resize(width, height);
}

void nex::Texture::setImpl(TextureImpl* impl)
{
	mImpl = impl;
}

nex::Texture2D::Texture2D(TextureImpl* impl) : Texture(impl)
{
}

void nex::TextureImpl::resize(unsigned width, unsigned height)
{
}

nex::Texture::Texture(TextureImpl* impl) : mImpl(impl)
{
}
