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
	inline void VertexLayout::push<float>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];

		bufferLayout.attributes.push_back({ LayoutPrimitive::FLOAT, count, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * VertexAttribute::getSizeOfType(LayoutPrimitive::FLOAT);
		++mLocationCounter;
	}

	template <>
	inline void VertexLayout::push<unsigned>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];
		bufferLayout.attributes.push_back({ LayoutPrimitive::UNSIGNED_INT, count, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_INT);
		++mLocationCounter;
	}

	template <>
	inline void VertexLayout::push<unsigned char>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];
		bufferLayout.attributes.push_back({ LayoutPrimitive::UNSIGNED_BYTE, count, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_BYTE);
		++mLocationCounter;
	}

	template <>
	void VertexLayout::push<unsigned short>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];
		bufferLayout.attributes.push_back({ LayoutPrimitive::UNSIGNED_SHORT, count, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_SHORT);
		++mLocationCounter;
	}

	template <>
	inline void VertexLayout::push<glm::vec4>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];
		bufferLayout.attributes.push_back({ LayoutPrimitive::FLOAT, count * 4, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * 4 * VertexAttribute::getSizeOfType(LayoutPrimitive::FLOAT);
		++mLocationCounter;
	}

	template <>
	inline void VertexLayout::push<glm::vec3>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];
		bufferLayout.attributes.push_back({ LayoutPrimitive::FLOAT, count * 3, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * 3 * VertexAttribute::getSizeOfType(LayoutPrimitive::FLOAT);
		++mLocationCounter;
	}

	template <>
	inline void VertexLayout::push<glm::vec2>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];
		bufferLayout.attributes.push_back({ LayoutPrimitive::FLOAT, count * 2, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * 2 * VertexAttribute::getSizeOfType(LayoutPrimitive::FLOAT);
		++mLocationCounter;
	}

	template <>
	inline void VertexLayout::push<glm::uvec4>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];
		bufferLayout.attributes.push_back({ LayoutPrimitive::UNSIGNED_INT, count * 4, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * 4 * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_INT);
		++mLocationCounter;
	}

	template <>
	inline void VertexLayout::push<glm::uvec3>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];
		bufferLayout.attributes.push_back({ LayoutPrimitive::UNSIGNED_INT, count * 3, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * 3 * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_INT);
		++mLocationCounter;
	}

	template <>
	inline void VertexLayout::push<glm::uvec2>(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		auto& bufferLayout = mMap[buffer];
		bufferLayout.attributes.push_back({ LayoutPrimitive::UNSIGNED_INT, count * 2, mLocationCounter, normalized, instanced, convertToFloat });
		bufferLayout.stride += count * 2 * VertexAttribute::getSizeOfType(LayoutPrimitive::UNSIGNED_INT);
		++mLocationCounter;
	}



	inline const nex::VertexLayout::BufferLayoutMap& VertexLayout::getBufferLayoutMap() const {
		return mMap;
	}

	inline nex::VertexLayout::BufferLayoutMap& VertexLayout::getBufferLayoutMap() {
		return mMap;
	}

	inline const nex::BufferLayout* VertexLayout::getLayout(const GpuBuffer* buffer) const
	{
		auto it = mMap.find(buffer);
		if (it == mMap.end()) return nullptr;
		return &it->second;
	}

	inline nex::BufferLayout& VertexLayout::getLayout(const GpuBuffer* buffer) {
		return mMap[buffer];
	}

	template <typename T>
	inline void VertexLayout::push(unsigned count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat)
	{
		static_assert(false);
	}
}