#include <nex/texture/Texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <exception>
#include <stdexcept>
#include <nex/util/ExceptionHandling.hpp>

using namespace std;
using namespace glm;

glm::mat4 CubeMap::rightSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
glm::mat4 CubeMap::leftSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
glm::mat4 CubeMap::topSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
glm::mat4 CubeMap::bottomSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
glm::mat4 CubeMap::frontSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
glm::mat4 CubeMap::backSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f));


const mat4& CubeMap::getViewLookAtMatrixRH(Side side)
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
}