#pragma once
#include <nex/mesh/VertexBuffer.hpp>
#include <nex/mesh/VertexLayout.hpp>

namespace nex
{

	class VertexArray
	{
	public:
		VertexArray();
		VertexArray(VertexArray&& other) noexcept;
		VertexArray& operator=(VertexArray&& o) noexcept;

		VertexArray(const VertexArray& o) = delete;
		VertexArray& operator=(const VertexArray& o) = delete;

		~VertexArray();

		void addBuffer(VertexBuffer buffer, const VertexLayout& layout);

		void bind() const;
		void unbind() const;

	private:
		unsigned int mRendererID;
		std::vector<VertexBuffer> mBuffers;
	};
}