#pragma once

#include <nex/buffer/GpuBuffer.hpp>
#include <nex/mesh/MeshTypes.hpp>

namespace nex
{

	class IndexBuffer : public GpuBuffer
	{
	public:
		IndexBuffer(IndexElementType type, size_t count, const void* data = nullptr, UsageHint usage = UsageHint::STATIC_DRAW);

		IndexBuffer(UsageHint usage = UsageHint::STATIC_DRAW);

		IndexBuffer(IndexBuffer&& other) = default;
		IndexBuffer& operator=(IndexBuffer&& o) = default;

		IndexBuffer(const IndexBuffer& o) = delete;
		IndexBuffer& operator=(const IndexBuffer& o) = delete;

		virtual ~IndexBuffer();

		void fill(IndexElementType type, size_t count, const void* data, UsageHint usage = UsageHint::STATIC_DRAW);

		size_t getCount() const { return mCount; }
		IndexElementType getType() const { return mType; }

	private:
		size_t mCount = 0;
		IndexElementType mType;
	};
}