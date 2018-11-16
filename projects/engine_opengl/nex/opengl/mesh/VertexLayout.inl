#pragma once
#include "VertexLayout.hpp"

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
