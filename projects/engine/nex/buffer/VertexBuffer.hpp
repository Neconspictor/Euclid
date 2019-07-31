#pragma once

//TODO generalize buffers!
#include <nex/shader/ShaderBuffer.hpp>

namespace nex
{
	class VertexBuffer
	{
	public:

		VertexBuffer(const void* data, size_t size);

		VertexBuffer();


		VertexBuffer(VertexBuffer&& other) noexcept;
		VertexBuffer& operator=(VertexBuffer&& o) noexcept;

		VertexBuffer(const VertexBuffer& o) = delete;
		VertexBuffer& operator=(const VertexBuffer& o) = delete;

		~VertexBuffer();

		void bind();
		void unbind();

		/**
		 * Fills and resizes this buffer.
		 * Any previously hold data will be discarded.
		 */
		void fill(const void* data, size_t size, ShaderBuffer::UsageHint usage = ShaderBuffer::UsageHint::STATIC_DRAW);

		/**
		 * Note: bind() has to be called before calling this function. Otherwise this function behaviour is undefined.
		 */
		void* map(ShaderBuffer::UsageHint usage);

		/**
		 * Note: bind() has to be called before calling this function. Otherwise this function behaviour is undefined.
		 */
		void unmap();

		/**
		 * @return : The (byte) size of this buffer.
		 */
		size_t size() const;

	private:
		unsigned int mRendererID;
		size_t mSize;
	};
}