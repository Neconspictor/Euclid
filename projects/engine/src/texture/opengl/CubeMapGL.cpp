#include <texture/opengl/CubeMapGL.hpp>
#include <type_traits>

using namespace std;

CubeMapGL::CubeMapGL(GLuint cubeMap) : cubeMap(cubeMap)
{
}

CubeMapGL::CubeMapGL(const CubeMapGL& other)
{
	cubeMap = other.cubeMap;
}

CubeMapGL::CubeMapGL(CubeMapGL&& other)
{
	cubeMap = move(other.cubeMap);
}

CubeMapGL& CubeMapGL::operator=(const CubeMapGL& other)
{
	if (this == &other) return *this;
	cubeMap = other.cubeMap;
	return *this;
}

CubeMapGL& CubeMapGL::operator=(CubeMapGL&& other)
{
	if (this == &other) return *this;
	cubeMap = move(other.cubeMap);
	return *this;
}

CubeMapGL::~CubeMapGL()
{
}

GLuint CubeMapGL::getCubeMap() const
{
	return cubeMap;
}

void CubeMapGL::setCubeMap(GLuint id)
{
	cubeMap = id;
}