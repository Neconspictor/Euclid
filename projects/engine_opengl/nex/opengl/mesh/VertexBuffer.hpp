#pragma once
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

		void bind() const;
		void unbind() const;

		void fill(const void* data, size_t size);

	private:
		unsigned int mRendererID;
	};
}