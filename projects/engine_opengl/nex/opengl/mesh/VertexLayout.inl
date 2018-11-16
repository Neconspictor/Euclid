#pragma once
#include "VertexLayout.hpp"
#include <nex/opengl/renderer/RendererOpenGL.hpp>

inline unsigned LayoutElement::getSizeOfType(unsigned int type)
{
	switch (type)
	{
	case GL_FLOAT: return sizeof(GLfloat);
	case GL_UNSIGNED_INT: return sizeof(GLuint);
	case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
	default: throw std::runtime_error("Unsupported type: " + std::to_string(type));
	}

	ASSERT(false);
	return 0;
}

template <>
inline void VertexLayout::push<float>(unsigned count)
{
	mElements.push_back({GL_FLOAT, count, GL_FALSE});
	mStride += count * LayoutElement::getSizeOfType(GL_FLOAT);
}

template <>
inline void VertexLayout::push<unsigned>(unsigned count)
{
	mElements.push_back({GL_UNSIGNED_INT, count, GL_TRUE});
	mStride += count * LayoutElement::getSizeOfType(GL_UNSIGNED_INT);
}

template <>
inline void VertexLayout::push<unsigned char>(unsigned count)
{
	mElements.push_back({GL_UNSIGNED_BYTE, count, GL_FALSE});
	mStride += count * LayoutElement::getSizeOfType(GL_UNSIGNED_BYTE);
}

template <>
inline void VertexLayout::push<glm::vec3>(unsigned count)
{
	mElements.push_back({ GL_FLOAT, count * 3, GL_FALSE });
	mStride += count * 3 * LayoutElement::getSizeOfType(GL_FLOAT);
}

template <>
inline void VertexLayout::push<glm::vec2>(unsigned count)
{
	mElements.push_back({ GL_FLOAT, count * 2, GL_FALSE });
	mStride += count * 2 * LayoutElement::getSizeOfType(GL_FLOAT);
}

inline unsigned VertexLayout::getStride() const
{
	return mStride;
}

inline const std::vector<LayoutElement>& VertexLayout::getElements() const
{
	return mElements;
}

template <typename T>
inline void VertexLayout::push(unsigned count)
{
	static_assert(false);
}
