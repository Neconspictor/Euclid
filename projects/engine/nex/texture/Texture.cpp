#include <nex/texture/Texture.hpp>
#include <memory>
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
	case POSITIVE_X:
		return rightSide;
	case NEGATIVE_X:
		return leftSide;
	case POSITIVE_Y:
		return topSide;
	case NEGATIVE_Y:
		return bottomSide;
	case NEGATIVE_Z:
		return frontSide;
	case POSITIVE_Z:
		return backSide;
	default:
		throw_with_trace(std::runtime_error("No mapping defined for " + side));
	}

	// won't be reached
	return rightSide;
}

bool nex::isCubeTarget(TextureTarget target)
{
	auto first  = static_cast<unsigned>(TextureTarget::CUBE_POSITIVE_X);
	auto last = static_cast<unsigned>(TextureTarget::CUBE_NEGATIVE_Z);
	auto current = static_cast<unsigned>(target);
	return first <= current && last >= current;
}

nex::Texture::~Texture()
{
	delete mImpl;
	mImpl = nullptr;
}

nex::TextureImpl* nex::Texture::getImpl() const
{
	return mImpl;
}

unsigned nex::Texture::getHeight() const
{
	return height;
}

unsigned nex::Texture::getWidth() const
{
	return width;
}

void nex::Texture::setHeight(int height)
{
	this->height = height;
}

void nex::Texture::setWidth(int width)
{
	this->width = width;
}

nex::Texture::Texture(TextureImpl* impl) : mImpl(impl)
{
}
