#pragma once
#include <nex/mesh/VertexLayout.hpp>
#include <string>

namespace nex
{
	inline unsigned LayoutElement::getSizeOfType(LayoutType type)
	{
		switch (type)
		{
		case LayoutType::UNSIGNED_INT: return sizeof(unsigned int);
		case LayoutType::FLOAT: return sizeof(float);
		case LayoutType::UNSIGNED_BYTE: return sizeof(unsigned char);
		default: throw std::runtime_error("Unsupported type: " + std::to_string(type));
		}

		assert(false);
		return 0;
	}

	template <>
	inline void VertexLayout::push<float>(unsigned count)
	{
		mElements.push_back({ LayoutType::FLOAT, count, false});
		mStride += count * LayoutElement::getSizeOfType(LayoutType::FLOAT);
	}

	template <>
	inline void VertexLayout::push<unsigned>(unsigned count)
	{
		mElements.push_back({ LayoutType::UNSIGNED_INT, count, true });
		mStride += count * LayoutElement::getSizeOfType(LayoutType::UNSIGNED_INT);
	}

	template <>
	inline void VertexLayout::push<unsigned char>(unsigned count)
	{
		mElements.push_back({ LayoutType::UNSIGNED_BYTE, count, false });
		mStride += count * LayoutElement::getSizeOfType(LayoutType::UNSIGNED_BYTE);
	}

	template <>
	inline void VertexLayout::push<glm::vec3>(unsigned count)
	{
		mElements.push_back({ LayoutType::FLOAT, count * 3, false });
		mStride += count * 3 * LayoutElement::getSizeOfType(LayoutType::FLOAT);
	}

	template <>
	inline void VertexLayout::push<glm::vec2>(unsigned count)
	{
		mElements.push_back({ LayoutType::FLOAT, count * 2, false });
		mStride += count * 2 * LayoutElement::getSizeOfType(LayoutType::FLOAT);
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
}