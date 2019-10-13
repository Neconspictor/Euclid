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
		case LayoutType::UNSIGNED_SHORT: return sizeof(unsigned short);
		default: throw std::runtime_error("Unsupported type: " + std::to_string((unsigned)type));
		}

		assert(false);
		return 0;
	}

	template <>
	inline void VertexLayout::push<float>(unsigned count, bool instanced)
	{
		mElements.push_back({ LayoutType::FLOAT, count, false, instanced });
		mStride += count * LayoutElement::getSizeOfType(LayoutType::FLOAT);
	}

	template <>
	inline void VertexLayout::push<unsigned>(unsigned count, bool instanced)
	{
		mElements.push_back({ LayoutType::UNSIGNED_INT, count, true, instanced });
		mStride += count * LayoutElement::getSizeOfType(LayoutType::UNSIGNED_INT);
	}

	template <>
	inline void VertexLayout::push<unsigned char>(unsigned count, bool instanced)
	{
		mElements.push_back({ LayoutType::UNSIGNED_BYTE, count, true, instanced });
		mStride += count * LayoutElement::getSizeOfType(LayoutType::UNSIGNED_BYTE);
	}

	template <>
	void VertexLayout::push<unsigned short>(unsigned count, bool instanced)
	{
		mElements.push_back({ LayoutType::UNSIGNED_SHORT, count, true, instanced });
		mStride += count * LayoutElement::getSizeOfType(LayoutType::UNSIGNED_SHORT);
	}

	template <>
	inline void VertexLayout::push<glm::vec3>(unsigned count, bool instanced)
	{
		mElements.push_back({ LayoutType::FLOAT, count * 3, false, instanced });
		mStride += count * 3 * LayoutElement::getSizeOfType(LayoutType::FLOAT);
	}

	template <>
	inline void VertexLayout::push<glm::vec2>(unsigned count, bool instanced)
	{
		mElements.push_back({ LayoutType::FLOAT, count * 2, false, instanced });
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
	inline void VertexLayout::push(unsigned count, bool instanced)
	{
		static_assert(false);
	}
}