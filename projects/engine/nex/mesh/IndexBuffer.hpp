#pragma once

namespace nex
{

	enum class IndexElementType {
		BIT_16, FIRST = BIT_16,
		BIT_32, LAST = BIT_32,
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(const unsigned int* data, unsigned int count);

		IndexBuffer();

		IndexBuffer(IndexBuffer&& other) noexcept;
		IndexBuffer& operator=(IndexBuffer&& o) noexcept;

		IndexBuffer(const IndexBuffer& o) = delete;
		IndexBuffer& operator=(const IndexBuffer& o) = delete;

		~IndexBuffer();

		void bind() const;
		void unbind() const;

		void fill(const unsigned int* data, unsigned int count);
		void fill(const unsigned short* data, unsigned short count);

		unsigned int getCount() const { return mCount; }
		IndexElementType getType() const { return mType; }

	private:
		unsigned int mRendererID;
		unsigned int mCount = 0;
		IndexElementType mType;
	};
}
