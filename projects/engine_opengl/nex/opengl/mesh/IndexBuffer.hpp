#pragma once

namespace nex
{
	class IndexBuffer
	{
	public:
		IndexBuffer(const unsigned int* data, unsigned int count);

		IndexBuffer(IndexBuffer&& other) noexcept;
		IndexBuffer& operator=(IndexBuffer&& o) noexcept = default;

		IndexBuffer(const IndexBuffer& o) = delete;
		IndexBuffer& operator=(const IndexBuffer& o) = delete;

		~IndexBuffer();

		void bind() const;
		void unbind() const;

		inline unsigned int getCount() const { return mCount; }

	private:
		unsigned int mRendererID;
		unsigned int mCount;
	};
}