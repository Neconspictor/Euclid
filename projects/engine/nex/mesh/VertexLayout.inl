#pragma once
#include <nex/mesh/VertexLayout.hpp>
#include <string>

namespace nex
{
	inline unsigned VertexAttribute::getSizeOfType(LayoutPrimitive type)
	{
		switch (type)
		{
		case LayoutPrimitive::UNSIGNED_INT: return sizeof(unsigned int);
		case LayoutPrimitive::FLOAT: return sizeof(float);
		case LayoutPrimitive::UNSIGNED_BYTE: return sizeof(unsigned char);
		case LayoutPrimitive::UNSIGNED_SHORT: return sizeof(unsigned short);
		default: throw std::runtime_error("Unsupported type: " + std::to_string((unsigned)type));
		}

		assert(false);
		return 0;
	}

	template <>
	inline void VertexLayout::push<float>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::FLOAT, count, false, instanced, buffer });
		mStride += count * VertexAttribute::getSizeOfType(LayoutPrimitive::FLOAT);
	}

	template <>
	inline void VertexLayout::push<unsigned>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::UNSIGNED_INT, count, true, instanced, buffer });
		mStride += count * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_INT);
	}

	template <>
	inline void VertexLayout::push<unsigned char>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::UNSIGNED_BYTE, count, true, instanced, buffer });
		mStride += count * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_BYTE);
	}

	template <>
	void VertexLayout::push<unsigned short>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::UNSIGNED_SHORT, count, true, instanced, buffer });
		mStride += count * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_SHORT);
	}

	template <>
	inline void VertexLayout::push<glm::vec4>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::FLOAT, count * 4, false, instanced, buffer });
		mStride += count * 4 * VertexAttribute::getSizeOfType(LayoutPrimitive::FLOAT);
	}

	template <>
	inline void VertexLayout::push<glm::vec3>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::FLOAT, count * 3, false, instanced, buffer });
		mStride += count * 3 * VertexAttribute::getSizeOfType(LayoutPrimitive::FLOAT);
	}

	template <>
	inline void VertexLayout::push<glm::vec2>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::FLOAT, count * 2, false, instanced, buffer });
		mStride += count * 2 * VertexAttribute::getSizeOfType(LayoutPrimitive::FLOAT);
	}

	template <>
	inline void VertexLayout::push<glm::uvec4>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::UNSIGNED_INT, count * 4, false, instanced, buffer });
		mStride += count * 4 * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_INT);
	}

	template <>
	inline void VertexLayout::push<glm::uvec3>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::UNSIGNED_INT, count * 3, false, instanced, buffer });
		mStride += count * 3 * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_INT);
	}

	template <>
	inline void VertexLayout::push<glm::uvec2>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		mAttributes.push_back({ LayoutPrimitive::UNSIGNED_INT, count * 2, false, instanced, buffer });
		mStride += count * 2 * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_INT);
	}

	inline unsigned VertexLayout::getStride() const
	{
		return mStride;
	}

	inline const std::vector<VertexAttribute>& VertexLayout::getAttributes() const
	{
		return mAttributes;
	}

	inline std::vector<VertexAttribute>& VertexLayout::getAttributes() {
		return mAttributes;
	}

	template <typename T>
	inline void VertexLayout::push(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced)
	{
		static_assert(false);
	}
}