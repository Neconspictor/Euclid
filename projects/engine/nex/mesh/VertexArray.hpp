#pragma once

namespace nex
{
	class VertexBuffer;
	class VertexLayout;

	class VertexArray
	{
	public:
		VertexArray();
		VertexArray(VertexArray&& other) noexcept;
		VertexArray& operator=(VertexArray&& o) noexcept;

		VertexArray(const VertexArray& o) = delete;
		VertexArray& operator=(const VertexArray& o) = delete;

		~VertexArray();

		void useBuffer(const VertexBuffer& buffer, const VertexLayout& layout);

		void bind() const;

		void unbind() const;

	private:
		unsigned int mRendererID;
	};
}