#pragma once

#include <nex/mesh/VertexLayout.hpp>

namespace nex
{
	class VertexBuffer;
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

		VertexLayout& getLayout();
		const VertexLayout& getLayout() const;

		/**
		 * Initializes the vertex array with its layout.
		 */
		void init();

		void setLayout(const VertexLayout& layout);

		void unbind() const;

	private:

		void assign(const GpuBuffer*  buffer, const VertexLayout& layout);

		VertexLayout mLayout;
		unsigned int mRendererID;
	};
}