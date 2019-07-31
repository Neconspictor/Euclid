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

		void useBuffer(VertexBuffer& buffer, const VertexLayout& layout);

		void bind();

		void unbind();

	private:
		unsigned int mRendererID;
	};
}