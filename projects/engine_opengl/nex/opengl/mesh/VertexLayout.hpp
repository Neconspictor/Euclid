#pragma once
#include <vector>
#include "glad/glad.h"
#include <string>
#include "nex/opengl/renderer/RendererOpenGL.hpp"

struct LayoutElement
{
	unsigned int type;
	unsigned int count;
	unsigned char normalized;

	static unsigned int getSizeOfType(unsigned int type)
	{
		switch (type)
		{
			case GL_FLOAT:			return sizeof(GLfloat);
			case GL_UNSIGNED_INT:	return sizeof(GLuint);
			case GL_UNSIGNED_BYTE:	return sizeof(GLubyte);
			default: throw std::runtime_error("Unsupported type: " + std::to_string(type));
		}

		ASSERT(false);
		return 0;
	}

};

class VertexLayout
{
private:
	std::vector<LayoutElement> mElements;
	unsigned int mStride;
	
public:
	VertexLayout() : mStride(0) {}

	template<typename T>
	inline void push(unsigned int count);

	template<>
	inline void push<float>(unsigned int count);

	template<>
	inline void push<unsigned int>(unsigned int count);

	template<>
	inline void push<unsigned char>(unsigned int count);

	inline unsigned int getStride() const;
	inline const std::vector<LayoutElement>& getElements() const;
};

#include "VertexLayout.inl"
