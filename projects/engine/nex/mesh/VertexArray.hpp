#pragma once

namespace nex
{
	class VertexBuffer;
	class VertexLayout;
	class GpuBuffer;

	class VertexArray
	{
	public:
		VertexArray();
		VertexArray(VertexArray&& other) noexcept;
		VertexArray& operator=(VertexArray&& o) noexcept;

		VertexArray(const VertexArray& o) = delete;
		VertexArray& operator=(const VertexArray& o) = delete;

		~VertexArray();

		void bind() const;

		/**
		 * Initializes the vertex array with a given layout.
		 *
		 * NOTE: bind() has to be called priorly!
		 */
		void init(const VertexLayout& layout);

		void unbind() const;

	private:

		void assign(const GpuBuffer*  buffer, const VertexLayout& layout);

		unsigned int mRendererID;
	};
}