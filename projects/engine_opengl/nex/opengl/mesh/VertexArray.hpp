#pragma once
#include "VertexBuffer.hpp"
#include "VertexLayout.hpp"

class VertexArray
{
public:
	VertexArray();
	VertexArray(VertexArray&& other) noexcept;
	VertexArray& operator=(VertexArray&& o) noexcept = default;

	VertexArray(const VertexArray& o) = delete;
	VertexArray& operator=(const VertexArray& o) = delete;

	~VertexArray();

	void addBuffer(const VertexBuffer& buffer, const VertexLayout& layout);

	void bind() const;
	void unbind() const;

private:
	unsigned int mRendererID;
};