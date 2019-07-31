#pragma once

#include <nex/shader/ShaderBuffer.hpp>
#include <nex/mesh/MeshTypes.hpp>

namespace nex
{

	class IndexBuffer
	{
	public:
		IndexBuffer(const void* data, size_t count, IndexElementType type);

		IndexBuffer();

		IndexBuffer(IndexBuffer&& other) noexcept;
		IndexBuffer& operator=(IndexBuffer&& o) noexcept;

		IndexBuffer(const IndexBuffer& o) = delete;
		IndexBuffer& operator=(const IndexBuffer& o) = delete;

		~IndexBuffer();

		void bind();
		void unbind();

		void fill(const void* data, size_t count, IndexElementType type, ShaderBuffer::UsageHint usage = ShaderBuffer::UsageHint::STATIC_DRAW);

		size_t getCount() const { return mCount; }
		IndexElementType getType() const { return mType; }

	private:
		unsigned int mRendererID;
		size_t mCount = 0;
		IndexElementType mType;
	};
}
