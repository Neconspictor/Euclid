#ifndef ENGINE_TEXTURE_OPENGL_CUBEMAPGL_HPP
#define ENGINE_TEXTURE_OPENGL_CUBEMAPGL_HPP
#include <texture/CubeMap.hpp>
#include <GL/glew.h>


class CubeMapGL : public CubeMap
{
public:
	CubeMapGL(GLuint cubeMap);
	CubeMapGL(const CubeMapGL& other);
	CubeMapGL(CubeMapGL&& other);
	CubeMapGL& operator=(const CubeMapGL& other);
	CubeMapGL& operator=(CubeMapGL&& other);

	virtual ~CubeMapGL();

	GLuint getCubeMap() const;

	void setCubeMap(GLuint id);

private:
	GLuint cubeMap;
};
#endif