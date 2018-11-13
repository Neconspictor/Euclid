#include <nex/opengl/mesh/MeshGL.hpp>

using namespace std;

MeshGL::MeshGL() : vao(GL_FALSE), vbo(GL_FALSE), ebo(GL_FALSE), indexSize(0)
{
}

MeshGL::MeshGL(MeshGL&& o) : indexSize(o.indexSize), vao(o.vao), vbo(o.vbo), ebo(o.ebo)
{
	material = move(o.material);
	o.vao = GL_FALSE;
	o.vbo = GL_FALSE;
	o.ebo = GL_FALSE;
}

MeshGL& MeshGL::operator=(MeshGL&& o)
{
	if (this == &o) return *this;
	
	indexSize = o.indexSize; 
	vao = o.vao; 
	vbo = o.vbo; 
	ebo = o.ebo;
	material = move(o.material);
	o.vao = GL_FALSE;
	o.vbo = GL_FALSE;
	o.ebo = GL_FALSE;
	
	return *this;
}

MeshGL::~MeshGL()
{
	glDeleteVertexArrays(1, &vao);
	vao = GL_FALSE;

	glDeleteBuffers(1, &vbo);
	vbo = GL_FALSE;

	glDeleteBuffers(1, &ebo);
	ebo = GL_FALSE;
}

GLuint MeshGL::getVertexArrayObject() const
{
	return vao;
}

GLuint MeshGL::getVertexBufferObject() const
{
	return vbo;
}

GLuint MeshGL::getElementBufferObject() const
{
	return ebo;
}

void MeshGL::setVertexArrayObject(GLuint vao)
{
	glDeleteVertexArrays(1, &this->vao);
	this->vao = vao;
}

void MeshGL::setVertexBufferObject(GLuint vbo)
{
	glDeleteBuffers(1, &this->vbo);
	this->vbo = vbo;
}

void MeshGL::setElementBufferObject(GLuint ebo)
{
	glDeleteBuffers(1, &this->ebo);
	this->ebo = ebo;
}

reference_wrapper<Material> MeshGL::getMaterial() const
{
	return std::ref(*material);
}

void MeshGL::setIndexSize(uint32_t indexSize)
{
	this->indexSize = indexSize;
}

void MeshGL::setMaterial(std::unique_ptr<Material> material)
{
	this->material = move(material);
}

unsigned MeshGL::getIndexSize() const
{
	return indexSize;
}