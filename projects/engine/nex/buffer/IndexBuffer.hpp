#pragma once

#include <nex/buffer/GpuBuffer.hpp>
#include <nex/mesh/MeshTypes.hpp>

namespace nex
{

	class IndexBuffer : public GpuBuffer
	{
	public:
		IndexBuffer(const void* data, size_t count, IndexElementType type);

		IndexBuffer();

		IndexBuffer(IndexBuffer&& other) = default;
		IndexBuffer& operator=(IndexBuffer&& o) = default;

		IndexBuffer(const IndexBuffer& o) = delete;
		IndexBuffer& operator=(const IndexBuffer& o) = delete;

		virtual ~IndexBuffer();

		void bind();
		void unbind();

		void fill(const void* data, size_t count, IndexElementType type, UsageHint usage = UsageHint::STATIC_DRAW);

		size_t getCount() const { return mCount; }
		IndexElementType getType() const { return mType; }

	private:
		size_t mCount = 0;
		IndexElementType mType;
	};
}
